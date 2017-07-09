#include "EventLoop.h"

class CurlContext
{
public:
    CurlContext(
        uv_loop_t* uv_loop,
        curl_socket_t sock_fd
    ) :
        m_sock_fd(sock_fd)
    {
        uv_poll_init_socket(uv_loop, &m_poll_handle, m_sock_fd);
        m_poll_handle.data = this;
    }

    CurlContext(const CurlContext& copy) = delete;
    CurlContext(CurlContext&& move) = default;
    auto operator=(const CurlContext& assign) = delete;

    auto Close()
    {
        uv_poll_stop(&m_poll_handle);
        /**
         * uv requires us to jump through a few hoops before we can delete ourselves.
         */
        uv_close((uv_handle_t*)&m_poll_handle, CurlContext::on_close);
    }

    auto GetPollHandle() -> uv_poll_t*
    {
        return &m_poll_handle;
    }
    auto GetCurlSocket() -> curl_socket_t
    {
        return m_sock_fd;
    }

private:
    uv_poll_t m_poll_handle;
    curl_socket_t m_sock_fd;

    static auto on_close(uv_handle_t* handle) -> void
    {
        CurlContext* curl_context = static_cast<CurlContext*>(handle->data);
        /**
         * uv has signaled that it is finished with the m_poll_handle,
         * we can now safely delete 'this'.
         */
        delete curl_context;
    }
};

static auto curl_start_timeout(
    CURLM* cmh,
    long timeout_ms,
    void* user_data
) -> void;

static auto curl_handle_socket(
    CURL* curl,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp
) -> int;

EventLoop::EventLoop() :
    m_loop(uv_default_loop()),
    m_cmh(curl_multi_init())
{
    uv_timer_init(m_loop, &m_timeout_timer);

    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETFUNCTION, curl_handle_socket);
    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERFUNCTION, curl_start_timeout);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERDATA, this);
}

EventLoop::~EventLoop() {
    uv_loop_delete(m_loop);
    curl_multi_cleanup(m_cmh);
}

auto EventLoop::Run() -> void {
    uv_run(m_loop, UV_RUN_DEFAULT);
}

auto EventLoop::GetUVLoop() -> uv_loop_t*
{
    return m_loop;
}

auto EventLoop::GetUVTimeoutTimer() -> uv_timer_t*
{
    return &m_timeout_timer;
}

auto EventLoop::GetCurlMultiHandle() -> CURLM* {
    return m_cmh;
}

auto EventLoop::OnUvTimeout(
    uv_timer_t* /*handle*/,
    int /*status*/
) {
    int running_handles;
    curl_multi_socket_action(m_cmh, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    checkMultiInfo();
}

auto EventLoop::checkMultiInfo() -> void {
    CURLMsg* message;
    int pending;

    while((message = curl_multi_info_read(m_cmh, &pending)))
    {
        switch(message->msg)
        {
            case CURLMSG_DONE:
            {
                char* url = nullptr;
                curl_easy_getinfo(
                    message->easy_handle,
                    CURLINFO_EFFECTIVE_URL,
                    &url
                );
                curl_multi_remove_handle(m_cmh, message->easy_handle);
                curl_easy_cleanup(message->easy_handle);
                break;
            }
            default:
                break;
        }
    }
}

auto EventLoop::CurlPerform(
    uv_poll_t* req,
    int status,
    int events
) -> void {
    uv_timer_stop(&m_timeout_timer);

    int running_handles;
    int flags = 0;
    if(status < 0)
    {
        flags = CURL_CSELECT_ERR;
    }
    if(!status && (events & UV_READABLE))
    {
        flags |= CURL_CSELECT_IN;
    }
    if(!status && (events & UV_WRITABLE))
    {
        flags |= CURL_CSELECT_OUT;
    }

    CurlContext* curl_context = static_cast<CurlContext*>(req->data);

    curl_multi_socket_action(m_cmh, curl_context->GetCurlSocket(), flags, &running_handles);
    checkMultiInfo();
}

static auto curl_start_timeout(
    CURLM* /*cmh*/,
    long timeout_ms,
    void* user_data
) -> void
{
    EventLoop* event_loop = static_cast<EventLoop*>(user_data);

    if(timeout_ms <= 0)
    {
        timeout_ms = 1;
    }
    uv_timer_start(
        event_loop->GetUVTimeoutTimer(),
        event_loop->OnUvTimeout,
        static_cast<uint64_t>(timeout_ms),
        0
    );
}

static auto curl_handle_socket(
    CURL* /*curl*/,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp
) -> int
{
    EventLoop* event_loop = static_cast<EventLoop*>(user_data);

    CurlContext* curl_context = nullptr;
    if(action == CURL_POLL_IN || action == CURL_POLL_OUT)
    {
        if(socketp != nullptr)
        {
            // existing request
            curl_context = static_cast<CurlContext*>(socketp);
        }
        else
        {
            // new request
            curl_context = new CurlContext(event_loop->GetUVLoop(), socket);
            curl_multi_assign(event_loop->GetCurlMultiHandle(), socket, static_cast<void*>(curl_context));
        }
    }

    switch(action)
    {
        case CURL_POLL_IN:
            uv_poll_start(curl_context->GetPollHandle(), UV_READABLE, event_loop->CurlPerform);
            break;
        case CURL_POLL_OUT:
            uv_poll_start(curl_context->GetPollHandle(), UV_WRITABLE, event_loop->CurlPerform);
            break;
        case CURL_POLL_REMOVE:
            if(socketp != nullptr)
            {
                curl_context = static_cast<CurlContext*>(socketp);
                curl_context->Close(); // signal this handle is done
                curl_multi_assign(event_loop->GetCurlMultiHandle(), socket, nullptr);
            }
            break;
        default:
            break;
    }

    return 0;
}
