#include "lift/client.hpp"
#include "lift/init.hpp"

#include <curl/curl.h>
#include <curl/multi.h>

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace lift
{
template<typename output_type, typename input_type>
static auto uv_type_cast(input_type* i) -> output_type*
{
    auto* void_ptr = static_cast<void*>(i);
    return static_cast<output_type*>(void_ptr);
}

class curl_context
{
public:
    explicit curl_context(client& c) : m_client(c) { m_poll_handle.data = this; }

    ~curl_context() = default;

    curl_context(const curl_context&)                             = delete;
    curl_context(curl_context&&)                                  = delete;
    auto operator=(const curl_context&) noexcept -> curl_context& = delete;
    auto operator=(curl_context&&) noexcept -> curl_context&      = delete;

    auto init(uv_loop_t* uv_loop, curl_socket_t sock_fd) -> void
    {
        m_sock_fd = sock_fd;
#if defined(_WIN32) && !defined(__LWIP_OPT_H__) && !defined(LWIP_HDR_OPT_H)
        uv_poll_init_socket(uv_loop, &m_poll_handle, m_sock_fd);
#else
        uv_poll_init(uv_loop, &m_poll_handle, m_sock_fd);
#endif
    }

    auto close()
    {
        uv_poll_stop(&m_poll_handle);
        /**
         * uv requires us to jump through a few hoops before we can delete ourselves.
         */
        uv_close(uv_type_cast<uv_handle_t>(&m_poll_handle), curl_context::on_close);
    }

    inline auto lift_client() -> client& { return m_client; }
    inline auto uv_poll_handle() -> uv_poll_t& { return m_poll_handle; }
    inline auto curl_sock_fd() -> curl_socket_t { return m_sock_fd; }

    static auto on_close(uv_handle_t* handle) -> void
    {
        auto* cc = static_cast<curl_context*>(handle->data);
        /**
         * uv has signaled that it is finished with the m_poll_handle,
         * we can now safely tell the event loop to re-use this curl context.
         */
        cc->lift_client().m_curl_context_ready.emplace_back(cc);
    }

private:
    client&       m_client;
    uv_poll_t     m_poll_handle{};
    curl_socket_t m_sock_fd{CURL_SOCKET_BAD};
};

auto curl_start_timeout(CURLM* cmh, long timeout_ms, void* user_data) -> void;

auto curl_handle_socket_actions(CURL* curl, curl_socket_t socket, int action, void* user_data, void* socketp) -> int;

auto uv_close_callback(uv_handle_t* handle) -> void;

auto on_uv_timeout_callback(uv_timer_t* handle) -> void;

auto on_uv_curl_perform_callback(uv_poll_t* req, int status, int events) -> void;

auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

auto on_uv_shutdown_async(uv_async_t* handle) -> void;

auto on_uv_timesup_callback(uv_timer_t* handle) -> void;

client::client(options opts)
    : m_connect_timeout(std::move(opts.connect_timeout)),
      m_curl_context_ready(),
      m_resolve_hosts(std::move(opts.resolve_hosts).value_or(std::vector<resolve_host>{})),
      m_on_thread_callback(std::move(opts.on_thread_callback))
{
    global_init();

    for (std::size_t i = 0; i < opts.reserve_connections.value_or(0); ++i)
    {
        m_executors.push_back(executor::make_unique(this));
    }

    uv_loop_init(&m_uv_loop);

    uv_async_init(&m_uv_loop, &m_uv_async, on_uv_requests_accept_async);
    m_uv_async.data = this;

    uv_async_init(&m_uv_loop, &m_uv_async_shutdown_pipe, on_uv_shutdown_async);
    m_uv_async_shutdown_pipe.data = this;

    uv_timer_init(&m_uv_loop, &m_uv_timer_curl);
    m_uv_timer_curl.data = this;

    uv_timer_init(&m_uv_loop, &m_uv_timer_timeout);
    m_uv_timer_timeout.data = this;

    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETFUNCTION, curl_handle_socket_actions);
    curl_multi_setopt(m_cmh, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERFUNCTION, curl_start_timeout);
    curl_multi_setopt(m_cmh, CURLMOPT_TIMERDATA, this);

    if (opts.max_connections.has_value())
    {
        curl_multi_setopt(m_cmh, CURLMOPT_MAXCONNECTS, static_cast<long>(opts.max_connections.value()));
    }

    m_background_thread = std::thread{[this] { run(); }};

    /**
     * 'Spin' wait for the thread to spin-up and run the event loop,
     * this means when the constructor returns the user can start adding requests
     * immediately without waiting.
     */
    while (!is_running())
    {
        std::this_thread::yield();
    }
}

client::~client()
{
    m_is_stopping.exchange(true, std::memory_order_release);

    // Block until all requests are completed.
    while (!empty())
    {
        std::this_thread::sleep_for(1ms);
    }

    curl_multi_cleanup(m_cmh);

    // This breaks the main UV_RUN_DEFAULT loop.
    uv_stop(&m_uv_loop);
    // This tells the loop to cleanup all its resources.
    uv_async_send(&m_uv_async_shutdown_pipe);

    // Wait for the loop to finish cleaning up before closing up shop.
    while (uv_loop_alive(&m_uv_loop) > 0)
    {
        std::this_thread::sleep_for(1ms);
    }

    uv_loop_close(&m_uv_loop);

    m_background_thread.join();
    m_executors.clear();

    global_cleanup();
}

auto client::start_request(request_ptr&& request_ptr) -> request::async_future_type
{
    if (request_ptr == nullptr)
    {
        throw std::runtime_error{"lift::client::start_request (future) The request_ptr cannot be nullptr."};
    }

    auto future = request_ptr->async_future();
    start_request_common(std::move(request_ptr));
    return future;
}

auto client::start_request(request_ptr&& request_ptr, request::async_callback_type callback) -> void
{
    if (request_ptr == nullptr)
    {
        throw std::runtime_error{"lift::client::start_request (callback) The request_ptr cannot be nullptr."};
    }
    if (callback == nullptr)
    {
        throw std::runtime_error{"lift::client::start_request (callback) The callback cannot be nullptr."};
    }

    request_ptr->async_callback(std::move(callback));
    start_request_common(std::move(request_ptr));
}

auto client::start_request_common(request_ptr&& request_ptr) -> void
{
    if (m_is_stopping.load(std::memory_order_acquire))
    {
        start_request_notify_failed_start(std::move(request_ptr));
        return;
    }

    // Do this now so that the event loop takes into account 'pending' requests as well.
    m_active_request_count.fetch_add(1, std::memory_order_release);

    {
        std::lock_guard<std::mutex> guard{m_pending_requests_lock};
        m_pending_requests.emplace_back(std::move(request_ptr));
    }
    uv_async_send(&m_uv_async);
}

auto client::run() -> void
{
    if (m_on_thread_callback != nullptr)
    {
        m_on_thread_callback();
    }

    m_is_running.exchange(true, std::memory_order_release);
    if (uv_run(&m_uv_loop, UV_RUN_DEFAULT) > 0)
    {
        // Run until all uv_handle_t objects are cleaned up.
        while (uv_run(&m_uv_loop, UV_RUN_NOWAIT) > 0)
        {
            std::this_thread::sleep_for(1ms);
        }
    }

    m_is_running.exchange(false, std::memory_order_release);

    if (m_on_thread_callback != nullptr)
    {
        m_on_thread_callback();
    }
}

auto client::check_actions() -> void
{
    check_actions(CURL_SOCKET_TIMEOUT, 0);
}

auto client::check_actions(curl_socket_t socket, int event_bitmask) -> void
{
    int       running_handles = 0;
    CURLMcode curl_code       = CURLM_OK;
    do
    {
        curl_code = curl_multi_socket_action(m_cmh, socket, event_bitmask, &running_handles);
    } while (curl_code == CURLM_CALL_MULTI_PERFORM);

    CURLMsg* message   = nullptr;
    int      msgs_left = 0;

    while ((message = curl_multi_info_read(m_cmh, &msgs_left)) != nullptr)
    {
        if (message->msg == CURLMSG_DONE)
        {
            CURL*    easy_handle = message->easy_handle;
            CURLcode easy_result = message->data.result;

            executor* exe = nullptr;
            curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &exe);
            executor_ptr executor_ptr{exe};

            // Remove the handle from curl multi since it is done processing.
            curl_multi_remove_handle(m_cmh, easy_handle);

            // Notify the user (if it hasn't already timed out) that the request is completed.
            // This will also return the executor to the pool for reuse.
            complete_request_normal(std::move(executor_ptr), easy_result);
        }
    }
}

auto client::complete_request_normal(executor_ptr exe_ptr, CURLcode curl_code) -> void
{
    auto& exe = *exe_ptr.get();

    if (exe.m_on_complete_handler_processed == false)
    {
        // Don't run this logic twice ever.
        exe.m_on_complete_handler_processed = true;
        // Since the request completed remove it from the timeout set if it is there.
        remove_timeout(exe);

        // Ownership over the async_handlers_type must be 'owned' here, otherwise when the request
        // is moved below it will move out from under us and cause a segfault due to the custom
        // 'copy_but_actually_move' object wrapper.
        auto on_complete_handler = std::move(exe.m_request_async->m_on_complete_handler.m_object).value();

        if (std::holds_alternative<request::async_callback_type>(on_complete_handler))
        {
            exe.copy_curl_to_response(curl_code);

            auto& callback = std::get<request::async_callback_type>(on_complete_handler);
            callback(std::move(exe.m_request_async), std::move(exe.m_response));
        }
        else if (std::holds_alternative<request::async_promise_type>(on_complete_handler))
        {
            exe.copy_curl_to_response(curl_code);

            auto& promise = std::get<request::async_promise_type>(on_complete_handler);
            promise.set_value(std::make_pair(std::move(exe.m_request_async), std::move(exe.m_response)));
        }
        // else do nothing for std::monostate, the user doesn't want to be notified or this request
        // has timedout but was allowed to finish establishing a connection.
    }

    return_executor(std::move(exe_ptr));
    m_active_request_count.fetch_sub(1, std::memory_order_release);
}

auto client::complete_request_timeout(executor& exe) -> void
{
    /**
     * NOTE: This function doesn't remove the request from the curl multi handle nor does it
     *       return the executor to the pool.  That is because this function is still allowing
     *       for curl to complete to establish http connections for extremely *low* timeout values.
     *
     *       This function expects that complete_request_normal will eventually be called by curl
     *       to properly cleanup and finish the request and allow for the http connection to fully
     *       establish.
     */

    // Call on complete if it exists and hasn't been called before.
    if (exe.m_on_complete_handler_processed == false)
    {
        // Don't run this logic twice ever.
        exe.m_on_complete_handler_processed = true;

        // Ownership over the async_handlers_type must be 'owned' here, otherwise when the request
        // is moved below it will move out from under us and cause a segfault due to the custom
        // 'copy_but_actually_move' object wrapper.
        auto on_complete_handler = std::move(exe.m_request_async->m_on_complete_handler.m_object).value();

        // Removing the timesup is done in the uv timesup callback so it can prune
        // every single item that has timesup'ed.  Doing it here will cause 1 item
        // per loop iteration to be removed if there are multiple items with the same timesup value.
        // remove_timeout(exe);

        if (std::holds_alternative<request::async_callback_type>(on_complete_handler))
        {
            auto copy = complete_request_timeout_common(exe);

            auto& callback = std::get<request::async_callback_type>(on_complete_handler);
            callback(std::move(copy), std::move(exe.m_response));
        }
        else if (std::holds_alternative<request::async_promise_type>(on_complete_handler))
        {
            auto copy = complete_request_timeout_common(exe);

            auto& promise = std::get<request::async_promise_type>(on_complete_handler);
            promise.set_value(std::make_pair(std::move(copy), std::move(exe.m_response)));
        }
        // else do nothing for std::monostate, the user doesn't want to be notified.
    }
}

auto client::complete_request_timeout_common(executor& exe) -> request_ptr
{
    exe.m_response.m_lift_status = lift::lift_status::timeout;
    exe.set_timesup_response(exe.m_request->timeout().value());

    // IMPORTANT! Copying here is required _OR_ shared ownership must be added as libcurl
    // maintains char* type pointers into the request data structure.  There is no guarantee
    // after moving into user land that it will stay alive long enough until curl finishes its
    // own timeout.  Shared ownership would most likely require locks as well since any curl
    // handle still in the curl multi handle could be mutated by curl at any moment, copying
    // seems far safer.

    // IMPORTANT! The request's async completion handler object is _MOVED_ from the original
    // object into the copied object.  After making this copy the original object must not invoke
    // any async handlers (future or callback).
    auto copy_ptr = std::make_unique<request>(*exe.m_request_async);
    return copy_ptr;
}

auto client::add_timeout(executor& exe) -> void
{
    auto* request = exe.m_request;
    if (request->timeout().has_value())
    {
        auto timeout = exe.m_request->timeout().value();

        std::optional<std::chrono::milliseconds> connect_timeout{std::nullopt};
        if (request->connect_timeout().has_value())
        {
            // Prefer the individual connect timeout over the event loop default.
            connect_timeout = request->connect_timeout().value();
        }
        else if (m_connect_timeout.has_value())
        {
            connect_timeout = m_connect_timeout;
        }

        if (connect_timeout.has_value())
        {
            if (connect_timeout.value() > timeout)
            {
                auto       now         = uv_now(&m_uv_loop);
                time_point tp          = now + static_cast<time_point>(timeout.count());
                exe.m_timeout_iterator = m_timeouts.emplace(tp, &exe);

                update_timeouts();

                curl_easy_setopt(
                    exe.m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(connect_timeout.value().count()));
            }
            else
            {
                // If the user set a longer timeout on the individual request, just let curl handle it.
                curl_easy_setopt(exe.m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout.count()));
            }
        }
        else
        {
            curl_easy_setopt(exe.m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout.count()));
        }
    }
}

auto client::remove_timeout(executor& exe) -> std::multimap<uint64_t, executor*>::iterator
{
    if (exe.m_timeout_iterator.has_value())
    {
        auto iter = exe.m_timeout_iterator.value();
        auto next = m_timeouts.erase(iter);
        exe.m_timeout_iterator.reset();

        // Anytime an item is removed the timesup timer might need to be adjusted.
        update_timeouts();

        return next;
    }
    else
    {
        // This is probably a logic error if the passed in executor doesn't
        // have a timeout, we'll update the timeouts to be sture we are triggering
        // at the correct next timeout and return the end() so as not to go into
        // an infinite loop if this is the first item.
        update_timeouts();
        return m_timeouts.end();
    }
}

auto client::update_timeouts() -> void
{
    // TODO only change if it needs to change, this will probably require
    // an iterator to the item just added or removed to properly skip
    // setting the timer every single time.

    if (!m_timeouts.empty())
    {
        auto now   = uv_now(&m_uv_loop);
        auto first = m_timeouts.begin()->first;

        // If the first item is already 'expired' setting the timer to zero
        // will trigger uv to call its callback on the next loop iteration.
        // Otherwise set the difference to how many milliseconds are between
        // first and now.
        uint64_t timer_value{0};
        if (first > now)
        {
            timer_value = first - now;
        }

        uv_timer_stop(&m_uv_timer_timeout);
        uv_timer_start(&m_uv_timer_timeout, on_uv_timesup_callback, timer_value, 0);
    }
    else
    {
        uv_timer_stop(&m_uv_timer_timeout);
    }
}

auto client::acquire_executor() -> std::unique_ptr<executor>
{
    std::unique_ptr<executor> executor_ptr{nullptr};

    if (!m_executors.empty())
    {
        executor_ptr = std::move(m_executors.back());
        m_executors.pop_back();
    }

    if (executor_ptr == nullptr)
    {
        executor_ptr = executor::make_unique(this);
    }

    return executor_ptr;
}

auto client::return_executor(std::unique_ptr<executor> executor_ptr) -> void
{
    executor_ptr->reset();
    m_executors.push_back(std::move(executor_ptr));
}

auto curl_start_timeout(CURLM* /*cmh*/, long timeout_ms, void* user_data) -> void
{
    auto* c = static_cast<client*>(user_data);

    // Stop the current timer regardless.
    uv_timer_stop(&c->m_uv_timer_curl);

    if (timeout_ms > 0)
    {
        uv_timer_start(&c->m_uv_timer_curl, on_uv_timeout_callback, static_cast<uint64_t>(timeout_ms), 0);
    }
    else if (timeout_ms == 0)
    {
        c->check_actions();
    }
}

auto curl_handle_socket_actions(CURL* /*curl*/, curl_socket_t socket, int action, void* user_data, void* socketp) -> int
{
    auto* c = static_cast<client*>(user_data);

    curl_context* cc = nullptr;
    if (action == CURL_POLL_IN || action == CURL_POLL_OUT || action == CURL_POLL_INOUT)
    {
        if (socketp != nullptr)
        {
            // existing request
            cc = static_cast<curl_context*>(socketp);
        }
        else
        {
            // new request, and no curl context's available? make one
            if (c->m_curl_context_ready.empty())
            {
                auto curl_context_ptr = std::make_unique<curl_context>(*c);
                cc                    = curl_context_ptr.release();
            }
            else
            {
                cc = c->m_curl_context_ready.front().release();
                c->m_curl_context_ready.pop_front();
            }

            cc->init(&c->m_uv_loop, socket);
            curl_multi_assign(c->m_cmh, socket, static_cast<void*>(cc));
        }
    }

    switch (action)
    {
        case CURL_POLL_IN:
            uv_poll_start(&cc->uv_poll_handle(), UV_READABLE, on_uv_curl_perform_callback);
            break;
        case CURL_POLL_OUT:
            uv_poll_start(&cc->uv_poll_handle(), UV_WRITABLE, on_uv_curl_perform_callback);
            break;
        case CURL_POLL_INOUT:
            uv_poll_start(&cc->uv_poll_handle(), UV_READABLE | UV_WRITABLE, on_uv_curl_perform_callback);
            break;
        case CURL_POLL_REMOVE:
            if (socketp != nullptr)
            {
                cc = static_cast<curl_context*>(socketp);
                cc->close(); // signal this handle is done
                curl_multi_assign(c->m_cmh, socket, nullptr);
            }
            break;
        default:
            break;
    }

    return 0;
}

auto uv_close_callback(uv_handle_t* /*handle*/) -> void
{
    // auto* c = static_cast<client*>(handle->data);
    // (void)c;

    // Currently nothing needs to be done since all handles the event loop uses
    // are allocated within the lift::client object and not separate on the heap.
}

auto on_uv_timeout_callback(uv_timer_t* handle) -> void
{
    auto* c = static_cast<client*>(handle->data);
    c->check_actions();
}

auto on_uv_curl_perform_callback(uv_poll_t* req, int status, int events) -> void
{
    auto* cc = static_cast<curl_context*>(req->data);
    auto& c  = cc->lift_client();

    int32_t action = 0;
    if (status < 0)
    {
        action = CURL_CSELECT_ERR;
    }
    if (status == 0)
    {
        if ((events & UV_READABLE) != 0)
        {
            action |= CURL_CSELECT_IN;
        }
        if ((events & UV_WRITABLE) != 0)
        {
            action |= CURL_CSELECT_OUT;
        }
    }

    c.check_actions(cc->curl_sock_fd(), action);
}

auto on_uv_requests_accept_async(uv_async_t* handle) -> void
{
    auto* c = static_cast<client*>(handle->data);

    /**
     * This lock must not have any "curl_*" functions called
     * while it is held, curl has its own internal locks and
     * it can cause a deadlock.  This means we intentionally swap
     * vectors before working on them so we have exclusive access
     * to the request objects on the client thread.
     */
    {
        std::lock_guard<std::mutex> guard{c->m_pending_requests_lock};
        // swap so we can release the lock as quickly as possible
        c->m_grabbed_requests.swap(c->m_pending_requests);
    }

    for (auto& request_ptr : c->m_grabbed_requests)
    {
        auto executor_ptr = c->acquire_executor();
        executor_ptr->start_async(std::move(request_ptr));
        executor_ptr->prepare();

        // This must be done before adding to the CURLM* object,
        // if not its possible a very fast request could complete
        // before this gets into the multi-map!
        c->add_timeout(*executor_ptr);

        auto curl_code = curl_multi_add_handle(c->m_cmh, executor_ptr->m_curl_handle);

        if (curl_code != CURLM_OK && curl_code != CURLM_CALL_MULTI_PERFORM)
        {
            /**
             * If curl_multi_add_handle fails then notify the user that the request failed to start
             * immediately.  This will return the just acquired executor back into the pool.
             */
            c->complete_request_normal(std::move(executor_ptr), CURLcode::CURLE_SEND_ERROR);
        }
        else
        {
            /**
             * Drop the unique_ptr safety around the request_ptr while it is being
             * processed by curl.  When curl is finished completing the request
             * it will be put back into a request object for the client to use.
             */
            (void)executor_ptr.release();

            /**
             * Immediately call curl's check action to get the current request moving.
             * Curl appears to have an internal queue and if it gets too long it might
             * drop requests.
             */
            c->check_actions();
        }
    }

    c->m_grabbed_requests.clear();
}

auto on_uv_shutdown_async(uv_async_t* handle) -> void
{
    auto* c = static_cast<client*>(handle->data);

    uv_timer_stop(&c->m_uv_timer_curl);
    uv_timer_stop(&c->m_uv_timer_timeout);
    uv_close(uv_type_cast<uv_handle_t>(&c->m_uv_timer_curl), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&c->m_uv_timer_timeout), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&c->m_uv_async), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&c->m_uv_async_shutdown_pipe), uv_close_callback);
}

auto on_uv_timesup_callback(uv_timer_t* handle) -> void
{
    auto* c       = static_cast<client*>(handle->data);
    auto& timesup = c->m_timeouts;

    if (timesup.empty())
    {
        return;
    }

    auto now = uv_now(&c->m_uv_loop);

    // While the items in the timesup map are <= now "timesup" them to the client.
    auto iter = timesup.begin();
    while (iter != timesup.end())
    {
        auto& [tp, exe] = *iter;
        if (tp > now)
        {
            // Everything past this point has more time to wait.
            break;
        }

        c->complete_request_timeout(*exe);
        iter = c->remove_timeout(*exe);
    }
}

} // namespace lift
