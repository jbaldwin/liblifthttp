#include "EventLoop.h"

#include <curl/multi.h>

#include <iostream>

class CurlContext
{
public:
    CurlContext(
        EventLoop& event_loop,
        uv_loop_t* uv_loop,
        curl_socket_t sock_fd
    )
        :
            m_event_loop(event_loop),
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

    auto GetEventLoop() -> EventLoop&
    {
        return m_event_loop;
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
    EventLoop& m_event_loop;
    uv_poll_t m_poll_handle;
    curl_socket_t m_sock_fd;

    static auto on_close(uv_handle_t* handle) -> void
    {
        auto* curl_context = static_cast<CurlContext*>(handle->data);
        /**
         * uv has signaled that it is finished with the m_poll_handle,
         * we can now safely delete 'this'.
         */
        delete curl_context;
    }
};

auto curl_start_timeout(
    CURLM* cmh,
    long timeout_ms,
    void* user_data
) -> void;

auto curl_handle_socket(
    CURL* curl,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp
) -> int;

auto requests_accept_async(
    uv_async_t* async,
    int status
) -> void;

auto on_uv_timeout_callback(
    uv_timer_t* handle,
    int status
) -> void;

auto on_uv_curl_perform_callback(
    uv_poll_t* req,
    int status,
    int events
) -> void;

auto on_uv_timeout_callback_2(
    uv_timer_t* handle,
    int status
) -> void
{
    auto* event_loop = static_cast<EventLoop*>(handle->data);

    int running_handles = 0;
    curl_multi_socket_action(event_loop->m_cmh, CURL_SOCKET_TIMEOUT, 0, &running_handles);
}

EventLoop::EventLoop(
    std::unique_ptr<IRequestCallbacks> request_callbacks
)
    :
        m_request_callbacks(std::move(request_callbacks)),
        m_loop(uv_default_loop()),
        m_cmh(curl_multi_init())
{
    uv_async_init(m_loop, &m_async, requests_accept_async);
    m_async.data = this;

    uv_timer_init(m_loop, &m_timeout_timer);
    m_timeout_timer.data = this;

    uv_timer_init(m_loop, &m_timer2);
    m_timer2.data = this;

    uv_timer_start(&m_timer2, on_uv_timeout_callback_2, 1, 1);

    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETFUNCTION, curl_handle_socket);
    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERFUNCTION, curl_start_timeout);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERDATA, this);
}

EventLoop::~EventLoop()
{
    uv_loop_delete(m_loop);
    curl_multi_cleanup(m_cmh);
}

auto EventLoop::Run() -> void
{
    std::cout << __FUNCTION__ << std::endl;
    uv_run(m_loop, UV_RUN_DEFAULT);
}

auto EventLoop::Stop() -> void
{
    std::cout << __FUNCTION__ << std::endl;
    uv_stop(m_loop);
}

auto EventLoop::AddRequest(
    std::unique_ptr<AsyncRequest> curl_request_ptr
) -> void {
    std::cout << __FUNCTION__ << std::endl;

    auto url = curl_request_ptr->GetUrl();
    {
        std::lock_guard<std::mutex> guard(m_pending_requests_lock);
        m_pending_requests.emplace_back(std::move(curl_request_ptr));
    }
    uv_async_send(&m_async);
}

auto EventLoop::GetRequestCallbacks() -> IRequestCallbacks& {
    return *m_request_callbacks;
}

auto EventLoop::GetRequestCallbacks() const -> const IRequestCallbacks& {
    return *m_request_callbacks;
}

auto EventLoop::checkMultiInfo() -> void {
    std::cout << __FUNCTION__ << std::endl;

    CURLMsg* message;
    int pending;

    while((message = curl_multi_info_read(m_cmh, &pending)))
    {
        switch(message->msg)
        {
            case CURLMSG_DONE:
            {
                std::cout << "CURLMSG_DONE" << std::endl;

                AsyncRequest* curl_request = nullptr;
                curl_easy_getinfo(
                    message->easy_handle,
                    CURLINFO_PRIVATE,
                    &curl_request
                );

                auto curl_request_ptr = std::move(*curl_request->m_active_requests_position);
                m_active_requests.erase(curl_request->m_active_requests_position);
                curl_multi_remove_handle(m_cmh, message->easy_handle);

                std::cout << "message->data.result = " << message->data.result << std::endl;

                switch(message->data.result)
                {
                    case CURLcode::CURLE_OPERATION_TIMEDOUT:
                        m_request_callbacks->OnTimeout(std::move(curl_request_ptr));
                        break;
                    case CURLcode::CURLE_COULDNT_RESOLVE_HOST:
                        m_request_callbacks->OnHostLookupFailure(std::move(curl_request_ptr));
                        break;
                    default:
                        m_request_callbacks->OnComplete(std::move(curl_request_ptr));
                        break;
                }
            }
            default:
                break;
        }
    }
}

auto curl_start_timeout(
    CURLM* /*cmh*/,
    long timeout_ms,
    void* user_data
) -> void
{
    std::cout << __FUNCTION__ << std::endl;

    auto* event_loop = static_cast<EventLoop*>(user_data);

    if(timeout_ms <= 0)
    {
        int running_handles = 0;
        curl_multi_socket_action(event_loop->m_cmh, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    }
    else
    {
        uv_timer_start(
            &event_loop->m_timeout_timer,
            on_uv_timeout_callback,
            static_cast<uint64_t>(timeout_ms),
            0
        );
    }
}

auto on_uv_timeout_callback(
    uv_timer_t* handle,
    int status
) -> void
{
    std::cout << __FUNCTION__ << std::endl;

    auto* event_loop = static_cast<EventLoop*>(handle->data);
    int running_handles = 0;
    curl_multi_socket_action(event_loop->m_cmh, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    event_loop->checkMultiInfo();
}

auto on_uv_curl_perform_callback(
    uv_poll_t* req,
    int status,
    int events
) -> void
{
    std::cout << __FUNCTION__ << std::endl;

    auto* curl_context = static_cast<CurlContext*>(req->data);
    auto& event_loop = curl_context->GetEventLoop();
    uv_timer_stop(&event_loop.m_timeout_timer);

    int running_handles;
    int action = 0;
    if(status < 0)
    {
        action = CURL_CSELECT_ERR;
    }
    if(!status && (events & UV_READABLE))
    {
        action |= CURL_CSELECT_IN;
    }
    if(!status && (events & UV_WRITABLE))
    {
        action |= CURL_CSELECT_OUT;
    }

    curl_multi_socket_action(event_loop.m_cmh, curl_context->GetCurlSocket(), action, &running_handles);
    event_loop.checkMultiInfo();
}

auto curl_handle_socket(
    CURL* /*curl*/,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp
) -> int
{
    std::cout << __FUNCTION__ << std::endl;
    
    auto* event_loop = static_cast<EventLoop*>(user_data);

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
            curl_context = new CurlContext(*event_loop, event_loop->m_loop, socket);
            curl_multi_assign(event_loop->m_cmh, socket, static_cast<void*>(curl_context));
        }
    }

    switch(action)
    {
        case CURL_POLL_IN:
            uv_poll_start(curl_context->GetPollHandle(), UV_READABLE, on_uv_curl_perform_callback);
            break;
        case CURL_POLL_OUT:
            uv_poll_start(curl_context->GetPollHandle(), UV_WRITABLE, on_uv_curl_perform_callback);
            break;
        case CURL_POLL_REMOVE:
            if(socketp != nullptr)
            {
                curl_context = static_cast<CurlContext*>(socketp);
                curl_context->Close(); // signal this handle is done
                curl_multi_assign(event_loop->m_cmh, socket, nullptr);
            }
            break;
        default:
            break;
    }

    return 0;
}

auto requests_accept_async(
    uv_async_t* handle,
    int /*status*/
) -> void
{
    std::cout << __FUNCTION__ << std::endl;

    auto* event_loop = static_cast<EventLoop*>(handle->data);

    std::lock_guard<std::mutex> guard(event_loop->m_pending_requests_lock);
    for(auto& request : event_loop->m_pending_requests)
    {
        std::cout << "EventLoop::requests_accept_async() accept -> " << request->GetUrl() << std::endl;
        curl_multi_add_handle(event_loop->m_cmh, request->m_curl_handle);
        auto position = event_loop->m_active_requests.emplace(
            event_loop->m_active_requests.end(),
            std::move(request)
        );

        position->get()->m_active_requests_position = position;
    }

    // Swap to release memory instead of just clearing.
    std::vector<std::unique_ptr<AsyncRequest>> cleared;
    event_loop->m_pending_requests.swap(cleared);
}
