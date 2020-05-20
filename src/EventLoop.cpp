#include "lift/EventLoop.hpp"

#include <curl/curl.h>
#include <curl/multi.h>

#include <chrono>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

namespace lift {

template <typename O, typename I>
static auto uv_type_cast(I* i) -> O*
{
    auto* void_ptr = static_cast<void*>(i);
    return static_cast<O*>(void_ptr);
}

auto curl_share_lock(
    CURL* curl_ptr,
    curl_lock_data data,
    curl_lock_access access,
    void* user_ptr) -> void;

auto curl_share_unlock(
    CURL* curl_ptr,
    curl_lock_data data,
    void* user_ptr) -> void;

Share::Share(
    ShareOptions share_options)
{
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_LOCKFUNC, curl_share_lock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_UNLOCKFUNC, curl_share_unlock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_USERDATA, this);

    if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::DNS)) {
        curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    }

    if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::SSL)) {
        curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    }

    if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::DATA)) {
        curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
    }
}

Share::~Share()
{
    curl_share_cleanup(m_curl_share_ptr);
}

auto curl_share_lock(
    CURL*,
    curl_lock_data data,
    curl_lock_access,
    void* user_ptr) -> void
{
    auto& share = *static_cast<Share*>(user_ptr);

    if (data == CURL_LOCK_DATA_SHARE) {
        share.m_curl_share_all_lock.lock();
    } else if (data == CURL_LOCK_DATA_DNS) {
        share.m_curl_share_dns_lock.lock();
    } else if (data == CURL_LOCK_DATA_SSL_SESSION) {
        share.m_curl_share_ssl_lock.lock();
    } else if (data == CURL_LOCK_DATA_CONNECT) {
        share.m_curl_share_data_lock.lock();
    }
}

auto curl_share_unlock(
    CURL*,
    curl_lock_data data,
    void* user_ptr) -> void
{
    auto& share = *static_cast<Share*>(user_ptr);

    if (data == CURL_LOCK_DATA_SHARE) {
        share.m_curl_share_all_lock.unlock();
    } else if (data == CURL_LOCK_DATA_DNS) {
        share.m_curl_share_dns_lock.unlock();
    } else if (data == CURL_LOCK_DATA_SSL_SESSION) {
        share.m_curl_share_ssl_lock.unlock();
    } else if (data == CURL_LOCK_DATA_CONNECT) {
        share.m_curl_share_data_lock.unlock();
    }
}

class CurlContext {
public:
    explicit CurlContext(
        EventLoop& event_loop)
        : m_event_loop(event_loop)
    {
        m_poll_handle.data = this;
    }

    ~CurlContext() = default;

    CurlContext(const CurlContext&) = delete;
    CurlContext(CurlContext&&) = delete;
    auto operator=(const CurlContext&) noexcept -> CurlContext& = delete;
    auto operator=(CurlContext&&) noexcept -> CurlContext& = delete;

    auto Init(
        uv_loop_t* uv_loop,
        curl_socket_t sock_fd) -> void
    {
        m_sock_fd = sock_fd;
        uv_poll_init(uv_loop, &m_poll_handle, m_sock_fd);
    }

    auto Close()
    {
        uv_poll_stop(&m_poll_handle);
        /**
         * uv requires us to jump through a few hoops before we can delete ourselves.
         */
        uv_close(uv_type_cast<uv_handle_t>(&m_poll_handle), CurlContext::on_close);
    }

    inline auto GetEventLoop() -> EventLoop& { return m_event_loop; }
    inline auto GetUvPollHandle() -> uv_poll_t& { return m_poll_handle; }
    inline auto GetCurlSockFd() -> curl_socket_t { return m_sock_fd; }

    static auto on_close(
        uv_handle_t* handle) -> void
    {
        auto* curl_context = static_cast<CurlContext*>(handle->data);
        /**
         * uv has signaled that it is finished with the m_poll_handle,
         * we can now safely tell the event loop to re-use this curl context.
         */
        curl_context->m_event_loop.m_curl_context_ready.emplace_back(curl_context);
    }

private:
    EventLoop& m_event_loop;
    uv_poll_t m_poll_handle {};
    curl_socket_t m_sock_fd { CURL_SOCKET_BAD };
};

auto curl_start_timeout(
    CURLM* cmh,
    long timeout_ms,
    void* user_data) -> void;

auto curl_handle_socket_actions(
    CURL* curl,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp) -> int;

auto uv_close_callback(
    uv_handle_t* handle) -> void;

auto on_uv_timeout_callback(
    uv_timer_t* handle) -> void;

auto on_uv_curl_perform_callback(
    uv_poll_t* req,
    int status,
    int events) -> void;

auto on_uv_requests_accept_async(
    uv_async_t* handle) -> void;

auto on_uv_timesup_callback(
    uv_timer_t* handle) -> void;

EventLoop::EventLoop(
    std::optional<uint64_t> reserve_connections,
    std::optional<uint64_t> max_connections,
    std::optional<std::chrono::milliseconds> connection_time,
    std::vector<ResolveHost> resolve_hosts,
    std::shared_ptr<Share> share_ptr)
    : m_connection_time(std::move(connection_time))
    , m_resolve_hosts(std::move(resolve_hosts))
    , m_share_ptr(std::move(share_ptr))
{
    {
        std::lock_guard<std::mutex> guard { m_curl_handles_lock };
        for (std::size_t i = 0; i < reserve_connections.value_or(0); ++i) {
            m_curl_handles.push_back(curl_easy_init());
        }
    }

    uv_async_init(m_uv_loop, &m_async, on_uv_requests_accept_async);
    m_async.data = this;

    uv_timer_init(m_uv_loop, &m_timer_curl);
    m_timer_curl.data = this;

    uv_timer_init(m_uv_loop, &m_timer_timeout);
    m_timer_timeout.data = this;

    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETFUNCTION, curl_handle_socket_actions);
    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERFUNCTION, curl_start_timeout);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERDATA, this);

    if (max_connections.has_value()) {
        curl_multi_setopt(m_cmh, CURLMOPT_MAXCONNECTS, static_cast<long>(max_connections.value()));
    }

    m_background_thread = std::thread { [this] { run(); } };

    /**
     * Wait for the thread to spin-up and run the event loop,
     * this means when the constructor returns the user can start adding requests
     * immediately without waiting.
     */
    while (!IsRunning()) {
        std::this_thread::sleep_for(1ms);
    }
}

EventLoop::~EventLoop()
{
    m_is_stopping = true;

    while (ActiveRequestCount() > 0) {
        std::this_thread::sleep_for(1ms);
    }

    uv_timer_stop(&m_timer_curl);
    uv_timer_stop(&m_timer_timeout);
    uv_close(uv_type_cast<uv_handle_t>(&m_timer_curl), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&m_timer_timeout), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&m_async), uv_close_callback);

    while (!m_timer_curl_closed && !m_timer_timeout_closed && !m_async_closed) {
        std::this_thread::sleep_for(1ms);
        uv_async_send(&m_async); // fake a request to make sure the loop wakes up
    }
    uv_stop(m_uv_loop);

    m_background_thread.join();

    {
        std::lock_guard<std::mutex> guard { m_curl_handles_lock };
        for (auto* curl_handle : m_curl_handles) {
            curl_easy_cleanup(curl_handle);
        }
        m_curl_handles.clear();
    }

    curl_multi_cleanup(m_cmh);
    uv_loop_close(m_uv_loop);
}

auto EventLoop::IsRunning() -> bool
{
    return m_is_running;
}

auto EventLoop::Stop() -> void
{
    m_is_stopping = true;
}

auto EventLoop::ActiveRequestCount() const -> uint64_t
{
    return m_active_request_count;
}

auto EventLoop::StartRequest(
    RequestPtr request_ptr) -> bool
{
    if (m_is_stopping) {
        return false;
    }

    if (request_ptr == nullptr) {
        return false;
    }

    // Do this now so that the event loop takes into account 'pending' requests as well.
    ++m_active_request_count;

    // Prepare now since it won't block the event loop thread.
    auto executor_ptr = Executor::make(std::move(request_ptr), this);
    executor_ptr->prepare();
    {
        std::lock_guard<std::mutex> guard { m_pending_requests_lock };
        m_pending_requests.emplace_back(std::move(executor_ptr));
    }
    uv_async_send(&m_async);

    return true;
}

auto EventLoop::run() -> void
{
    m_tid = syscall(SYS_gettid);
    /**
     * Note that its possible to use m_background_thread.native_handle() however
     * gdb and other debugging tools have an issue reporting the thread id correctly, so
     * for now its better to call pthread_self().
     */
    m_native_handle = pthread_self();

    m_is_running = true;
    uv_run(m_uv_loop, UV_RUN_DEFAULT);
    m_is_running = false;
}

auto EventLoop::checkActions() -> void
{
    checkActions(CURL_SOCKET_TIMEOUT, 0);
}

auto EventLoop::checkActions(
    curl_socket_t socket,
    int event_bitmask) -> void
{
    int running_handles = 0;
    CURLMcode curl_code = CURLM_OK;
    do {
        curl_code = curl_multi_socket_action(m_cmh, socket, event_bitmask, &running_handles);
    } while (curl_code == CURLM_CALL_MULTI_PERFORM);

    CURLMsg* message = nullptr;
    int msgs_left = 0;

    while ((message = curl_multi_info_read(m_cmh, &msgs_left)) != nullptr) {
        if (message->msg == CURLMSG_DONE) {
            CURL* easy_handle = message->easy_handle;
            CURLcode easy_result = message->data.result;

            Executor* executor = nullptr;
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &executor);
            ExecutorPtr executor_ptr { executor };

            curl_multi_remove_handle(m_cmh, easy_handle);

            completeRequestNormal(
                *executor_ptr.get(),
                Executor::convert(easy_result));
        }
    }
}

auto EventLoop::completeRequestNormal(
    Executor& executor,
    LiftStatus status) -> void
{
    if (executor.m_on_complete_callback_called == false) {

        auto& on_complete_handler = executor.m_request_async->m_on_complete_handler;

        if (on_complete_handler != nullptr) {
            executor.m_on_complete_callback_called = true;
            executor.m_response.m_lift_status = status;
            executor.copyCurlToResponse();
            removeTimeout(executor);

            on_complete_handler(
                std::move(executor.m_request_async),
                std::move(executor.m_response));
        }
    }

    --m_active_request_count;
}

auto EventLoop::completeRequestTimeout(
    Executor& executor) -> void
{
    auto& on_complete_handler = executor.m_request_async->m_on_complete_handler;

    // Call on complete if it exists and hasn't been called before.
    if (executor.m_on_complete_callback_called == false && on_complete_handler != nullptr) {
        executor.m_on_complete_callback_called = true;
        executor.m_response.m_lift_status = lift::LiftStatus::TIMEOUT;
        executor.setTimesupResponse(executor.m_request->Timeout().value());

        // Removing the timesup is done in the uv timesup callback so it can prune
        // every single item that has timesup'ed.  Doing it here will cause 1 item
        // per loop iteration to be removed if there are multiple items with the same timesup value.
        // removeTimeout(executor);

        on_complete_handler(
            std::move(executor.m_request_async),
            std::move(executor.m_response));
    }

    // Lift timeouts do not trigger an active request count drop.
}

auto EventLoop::addTimeout(
    Executor& executor) -> void
{
    if (executor.m_request->Timeout().has_value()) {
        auto timeout = executor.m_request->Timeout().value();

        if (m_connection_time.has_value()) {
            auto connection_time = m_connection_time.value();

            if (connection_time > timeout) {
                auto now = uv_now(m_uv_loop);
                TimePoint time_point = now + timeout.count();
                executor.m_timeout_iterator = m_timeouts.emplace(time_point, &executor);

                updateTimeouts();

                curl_easy_setopt(
                    executor.m_curl_handle,
                    CURLOPT_TIMEOUT_MS,
                    static_cast<long>(connection_time.count()));
            } else {
                // If the user set a longer timeout on the individual request, just let curl handle it.
                curl_easy_setopt(
                    executor.m_curl_handle,
                    CURLOPT_TIMEOUT_MS,
                    static_cast<long>(timeout.count()));
            }
        } else {
            curl_easy_setopt(
                executor.m_curl_handle,
                CURLOPT_TIMEOUT_MS,
                static_cast<long>(timeout.count()));
        }
    }
}

auto EventLoop::removeTimeout(
    Executor& executor) -> std::multimap<uint64_t, Executor*>::iterator
{
    if (executor.m_timeout_iterator.has_value()) {
        auto iter = executor.m_timeout_iterator.value();
        auto next = m_timeouts.erase(iter);
        executor.m_timeout_iterator.reset();

        // Anytime an item is removed the timesup timer might need to be adjusted.
        updateTimeouts();

        return next;
    }

    return m_timeouts.end(); // is this behavior ok?
}

auto EventLoop::updateTimeouts() -> void
{
    // TODO only change if it needs to change, this will probably require
    // an iterator to the item just added or removed to properly skip
    // setting the timer every single time.

    if (!m_timeouts.empty()) {
        auto now = uv_now(m_uv_loop);
        auto first = m_timeouts.begin()->first;

        // If the first item is already 'expired' setting the timer to zero
        // will trigger uv to call its callback on the next loop iteration.
        // Otherwise set the difference to how many milliseconds are between
        // first and now.
        uint64_t timer_value { 0 };
        if (first > now) {
            timer_value = first - now;
        }

        uv_timer_stop(&m_timer_timeout);
        uv_timer_start(
            &m_timer_timeout,
            on_uv_timesup_callback,
            timer_value,
            0);
    } else {
        uv_timer_stop(&m_timer_timeout);
    }
}

auto EventLoop::acquireCurlHandle() -> CURL*
{
    CURL* curl_handle { nullptr };

    {
        std::lock_guard<std::mutex> guard { m_curl_handles_lock };
        if (!m_curl_handles.empty()) {
            curl_handle = m_curl_handles.back();
            m_curl_handles.pop_back();
        }
    }

    // Out of re-usable curl handles, create a new one outside the lock.
    if (curl_handle == nullptr) {
        curl_handle = curl_easy_init();
    }

    if (m_share_ptr != nullptr) {
        curl_easy_setopt(curl_handle, CURLOPT_SHARE, m_share_ptr->m_curl_share_ptr);
    }

    return curl_handle;
}

auto EventLoop::returnCurlHandle(CURL* curl_handle) -> void
{
    curl_easy_reset(curl_handle);
    {
        std::lock_guard<std::mutex> guard { m_curl_handles_lock };
        m_curl_handles.push_back(curl_handle);
    }
}

auto curl_start_timeout(
    CURLM* /*cmh*/,
    long timeout_ms,
    void* user_data) -> void
{
    auto* event_loop = static_cast<EventLoop*>(user_data);

    // Stop the current timer regardless.
    uv_timer_stop(&event_loop->m_timer_curl);

    if (timeout_ms > 0) {
        uv_timer_start(
            &event_loop->m_timer_curl,
            on_uv_timeout_callback,
            static_cast<uint64_t>(timeout_ms),
            0);
    } else if (timeout_ms == 0) {
        event_loop->checkActions();
    }
}

auto curl_handle_socket_actions(
    CURL* /*curl*/,
    curl_socket_t socket,
    int action,
    void* user_data,
    void* socketp) -> int
{
    auto* event_loop = static_cast<EventLoop*>(user_data);

    CurlContext* curl_context = nullptr;
    if (action == CURL_POLL_IN || action == CURL_POLL_OUT || action == CURL_POLL_INOUT) {
        if (socketp != nullptr) {
            // existing request
            curl_context = static_cast<CurlContext*>(socketp);
        } else {
            // new request, and no curl context's available? make one
            if (event_loop->m_curl_context_ready.empty()) {
                auto curl_context_ptr = std::make_unique<CurlContext>(*event_loop);
                curl_context = curl_context_ptr.release();
            } else {
                curl_context = event_loop->m_curl_context_ready.front().release();
                event_loop->m_curl_context_ready.pop_front();
            }

            curl_context->Init(event_loop->m_uv_loop, socket);
            curl_multi_assign(event_loop->m_cmh, socket, static_cast<void*>(curl_context));
        }
    }

    switch (action) {
    case CURL_POLL_IN:
        uv_poll_start(&curl_context->GetUvPollHandle(), UV_READABLE, on_uv_curl_perform_callback);
        break;
    case CURL_POLL_OUT:
        uv_poll_start(&curl_context->GetUvPollHandle(), UV_WRITABLE, on_uv_curl_perform_callback);
        break;
    case CURL_POLL_INOUT:
        uv_poll_start(&curl_context->GetUvPollHandle(), UV_READABLE | UV_WRITABLE, on_uv_curl_perform_callback);
        break;
    case CURL_POLL_REMOVE:
        if (socketp != nullptr) {
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

auto uv_close_callback(uv_handle_t* handle) -> void
{
    auto* event_loop = static_cast<EventLoop*>(handle->data);
    if (handle == uv_type_cast<uv_handle_t>(&event_loop->m_async)) {
        event_loop->m_async_closed = true;
    } else if (handle == uv_type_cast<uv_handle_t>(&event_loop->m_timer_curl)) {
        event_loop->m_timer_curl_closed = true;
    } else if (handle == uv_type_cast<uv_handle_t>(&event_loop->m_timer_timeout)) {
        event_loop->m_timer_timeout_closed = true;
    }
}

auto on_uv_timeout_callback(
    uv_timer_t* handle) -> void
{
    auto* event_loop = static_cast<EventLoop*>(handle->data);
    event_loop->checkActions();
}

auto on_uv_curl_perform_callback(
    uv_poll_t* req,
    int status,
    int events) -> void
{
    auto* curl_context = static_cast<CurlContext*>(req->data);
    auto& event_loop = curl_context->GetEventLoop();

    int32_t action = 0;
    if (status < 0) {
        action = CURL_CSELECT_ERR;
    }
    if (status == 0) {
        if ((events & UV_READABLE) != 0) {
            action |= CURL_CSELECT_IN;
        }
        if ((events & UV_WRITABLE) != 0) {
            action |= CURL_CSELECT_OUT;
        }
    }

    event_loop.checkActions(curl_context->GetCurlSockFd(), action);
}

auto on_uv_requests_accept_async(
    uv_async_t* handle) -> void
{
    auto* event_loop = static_cast<EventLoop*>(handle->data);

    /**
     * This lock must not have any "curl_*" functions called
     * while it is held, curl has its own internal locks and
     * it can cause a deadlock.  This means we intentionally swap
     * vectors before working on them so we have exclusive access
     * to the Request objects on the EventLoop thread.
     */
    {
        std::lock_guard<std::mutex> guard { event_loop->m_pending_requests_lock };
        // swap so we can release the lock as quickly as possible
        event_loop->m_grabbed_requests.swap(
            event_loop->m_pending_requests);
    }

    auto now = uv_now(event_loop->m_uv_loop);

    for (auto& executor_ptr : event_loop->m_grabbed_requests) {

        // This must be done before adding to the CURLM* object,
        // if not its possible a very fast request could complete
        // before this gets into the multi-map!
        event_loop->addTimeout(*executor_ptr);

        auto curl_code = curl_multi_add_handle(event_loop->m_cmh, executor_ptr->m_curl_handle);

        if (curl_code != CURLM_OK && curl_code != CURLM_CALL_MULTI_PERFORM) {
            /**
             * If curl_multi_add_handle fails then notify the user that the request failed to start
             * immediately.
             */
            event_loop->completeRequestNormal(
                *executor_ptr.get(),
                Executor::convert(CURLcode::CURLE_SEND_ERROR));
        } else {
            /**
             * Drop the unique_ptr safety around the RequestHandle while it is being
             * processed by curl.  When curl is finished completing the request
             * it will be put back into a Request object for the client to use.
             */
            (void)executor_ptr.release();

            /**
             * Immediately call curl's check action to get the current request moving.
             * Curl appears to have an internal queue and if it gets too long it might
             * drop requests.
             */
            event_loop->checkActions();
        }
    }

    event_loop->m_grabbed_requests.clear();
}

auto on_uv_timesup_callback(
    uv_timer_t* handle) -> void
{
    auto* event_loop = static_cast<EventLoop*>(handle->data);
    auto& timesup = event_loop->m_timeouts;

    if (timesup.empty()) {
        return;
    }

    auto now = uv_now(event_loop->m_uv_loop);

    // While the items in the timesup map are <= now "timesup" them to the client.
    auto iter = timesup.begin();
    while (iter != timesup.end()) {
        if (iter->first > now) {
            // Everything past this point has more time to wait.
            break;
        }

        auto& executor = *iter->second;
        event_loop->completeRequestTimeout(executor);
        iter = event_loop->removeTimeout(executor);
    }
}

} // lift
