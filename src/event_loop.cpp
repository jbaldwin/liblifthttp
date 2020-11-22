#include "lift/event_loop.hpp"
#include "lift/init.hpp"

#include <curl/curl.h>
#include <curl/multi.h>

#include <chrono>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>

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
    explicit curl_context(event_loop& event_loop) : m_event_loop(event_loop) { m_poll_handle.data = this; }

    ~curl_context() = default;

    curl_context(const curl_context&) = delete;
    curl_context(curl_context&&)      = delete;
    auto operator=(const curl_context&) noexcept -> curl_context& = delete;
    auto operator=(curl_context&&) noexcept -> curl_context& = delete;

    auto init(uv_loop_t* uv_loop, curl_socket_t sock_fd) -> void
    {
        m_sock_fd = sock_fd;
        uv_poll_init(uv_loop, &m_poll_handle, m_sock_fd);
    }

    auto close()
    {
        uv_poll_stop(&m_poll_handle);
        /**
         * uv requires us to jump through a few hoops before we can delete ourselves.
         */
        uv_close(uv_type_cast<uv_handle_t>(&m_poll_handle), curl_context::on_close);
    }

    inline auto lift_event_loop() -> event_loop& { return m_event_loop; }
    inline auto uv_poll_handle() -> uv_poll_t& { return m_poll_handle; }
    inline auto curl_sock_fd() -> curl_socket_t { return m_sock_fd; }

    static auto on_close(uv_handle_t* handle) -> void
    {
        auto* cc = static_cast<curl_context*>(handle->data);
        /**
         * uv has signaled that it is finished with the m_poll_handle,
         * we can now safely tell the event loop to re-use this curl context.
         */
        cc->lift_event_loop().m_curl_context_ready.emplace_back(cc);
    }

private:
    event_loop&   m_event_loop;
    uv_poll_t     m_poll_handle{};
    curl_socket_t m_sock_fd{CURL_SOCKET_BAD};
};

auto curl_start_timeout(CURLM* cmh, long timeout_ms, void* user_data) -> void;

auto curl_handle_socket_actions(CURL* curl, curl_socket_t socket, int action, void* user_data, void* socketp) -> int;

auto uv_close_callback(uv_handle_t* handle) -> void;

auto on_uv_timeout_callback(uv_timer_t* handle) -> void;

auto on_uv_curl_perform_callback(uv_poll_t* req, int status, int events) -> void;

auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

auto on_uv_timesup_callback(uv_timer_t* handle) -> void;

event_loop::event_loop(options opts)
    : m_connect_timeout(std::move(opts.connect_timeout)),
      m_curl_context_ready(),
      m_resolve_hosts(std::move(opts.resolve_hosts.value_or(std::vector<ResolveHost>{}))),
      m_share_ptr(std::move(opts.share_ptr)),
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
     * Spin wait for the thread to spin-up and run the event loop,
     * this means when the constructor returns the user can start adding requests
     * immediately without waiting.
     */
    while (!is_running()) {}
}

event_loop::~event_loop()
{
    m_is_stopping.exchange(true, std::memory_order_release);

    while (!empty())
    {
        std::this_thread::sleep_for(1ms);
    }

    uv_timer_stop(&m_uv_timer_curl);
    uv_timer_stop(&m_uv_timer_timeout);
    uv_close(uv_type_cast<uv_handle_t>(&m_uv_timer_curl), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&m_uv_timer_timeout), uv_close_callback);
    uv_close(uv_type_cast<uv_handle_t>(&m_uv_async), uv_close_callback);

    while (uv_loop_alive(&m_uv_loop))
    {
        std::this_thread::sleep_for(1ms);
        uv_async_send(&m_uv_async); // fake a request to make sure the loop wakes up
    }
    uv_stop(&m_uv_loop);
    uv_loop_close(&m_uv_loop);

    m_background_thread.join();
    m_executors.clear();

    curl_multi_cleanup(m_cmh);
    global_cleanup();
}

auto event_loop::start_request(RequestPtr request_ptr) -> bool
{
    if (request_ptr == nullptr)
    {
        return false;
    }

    if (m_is_stopping.load(std::memory_order_acquire))
    {
        return false;
    }

    // Do this now so that the event loop takes into account 'pending' requests as well.
    m_active_request_count.fetch_add(1, std::memory_order_relaxed);

    {
        std::lock_guard<std::mutex> guard{m_pending_requests_lock};
        m_pending_requests.emplace_back(std::move(request_ptr));
    }
    uv_async_send(&m_uv_async);

    return true;
}

auto event_loop::run() -> void
{
    if (m_on_thread_callback != nullptr)
    {
        m_on_thread_callback();
    }

    m_is_running.exchange(true, std::memory_order_release);
    uv_run(&m_uv_loop, UV_RUN_DEFAULT);
    m_is_running.exchange(false, std::memory_order_release);

    if (m_on_thread_callback != nullptr)
    {
        m_on_thread_callback();
    }
}

auto event_loop::check_actions() -> void
{
    check_actions(CURL_SOCKET_TIMEOUT, 0);
}

auto event_loop::check_actions(curl_socket_t socket, int event_bitmask) -> void
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

            curl_multi_remove_handle(m_cmh, easy_handle);

            complete_request_normal(*executor_ptr.get(), executor::convert(easy_result));

            return_executor(std::move(executor_ptr));
        }
    }
}

auto event_loop::complete_request_normal(executor& exe, LiftStatus status) -> void
{
    if (exe.m_on_complete_callback_called == false)
    {
        auto& on_complete_handler = exe.m_request_async->m_on_complete_handler;

        if (on_complete_handler != nullptr)
        {
            exe.m_on_complete_callback_called = true;
            exe.m_response.m_lift_status      = status;
            exe.copy_curl_to_response();
            remove_timeout(exe);

            on_complete_handler(std::move(exe.m_request_async), std::move(exe.m_response));
        }
    }

    m_active_request_count.fetch_sub(1, std::memory_order_relaxed);
}

auto event_loop::complete_request_timeout(executor& exe) -> void
{
    auto& on_complete_handler = exe.m_request_async->m_on_complete_handler;

    // Call on complete if it exists and hasn't been called before.
    if (exe.m_on_complete_callback_called == false && on_complete_handler != nullptr)
    {
        exe.m_on_complete_callback_called = true;
        exe.m_response.m_lift_status      = lift::LiftStatus::TIMEOUT;
        exe.set_timesup_response(exe.m_request->Timeout().value());

        // Removing the timesup is done in the uv timesup callback so it can prune
        // every single item that has timesup'ed.  Doing it here will cause 1 item
        // per loop iteration to be removed if there are multiple items with the same timesup value.
        // remove_timeout(exe);

        // IMPORTANT! Copying here is required _OR_ shared ownership must be added as libcurl
        // maintains char* type pointers into the request data structure.  There is no guarantee
        // after moving into user land that it will stay alive long enough until curl finishes its
        // own timeout.  Shared ownership would most likely require locks as well since any curl
        // handle still in the curl multi handle could be mutated by curl at any moment, copying
        // seems far safer.
        auto copy_ptr = std::make_unique<Request>(*exe.m_request_async);

        on_complete_handler(std::move(copy_ptr), std::move(exe.m_response));
    }

    // Lift timeouts do not trigger an active request count drop.
}

auto event_loop::add_timeout(executor& exe) -> void
{
    auto* request = exe.m_request;
    if (request->Timeout().has_value())
    {
        auto timeout = exe.m_request->Timeout().value();

        std::optional<std::chrono::milliseconds> connect_timeout{std::nullopt};
        if (request->ConnectTimeout().has_value())
        {
            // Prefer the individual connect timeout over the event loop default.
            connect_timeout = request->ConnectTimeout().value();
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

auto event_loop::remove_timeout(executor& exe) -> std::multimap<uint64_t, executor*>::iterator
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

auto event_loop::update_timeouts() -> void
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

auto event_loop::acquire_executor() -> std::unique_ptr<executor>
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

auto event_loop::return_executor(std::unique_ptr<executor> executor_ptr) -> void
{
    executor_ptr->reset();
    m_executors.push_back(std::move(executor_ptr));
}

auto curl_start_timeout(CURLM* /*cmh*/, long timeout_ms, void* user_data) -> void
{
    auto* el = static_cast<event_loop*>(user_data);

    // Stop the current timer regardless.
    uv_timer_stop(&el->m_uv_timer_curl);

    if (timeout_ms > 0)
    {
        uv_timer_start(&el->m_uv_timer_curl, on_uv_timeout_callback, static_cast<uint64_t>(timeout_ms), 0);
    }
    else if (timeout_ms == 0)
    {
        el->check_actions();
    }
}

auto curl_handle_socket_actions(CURL* /*curl*/, curl_socket_t socket, int action, void* user_data, void* socketp) -> int
{
    auto* el = static_cast<event_loop*>(user_data);

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
            if (el->m_curl_context_ready.empty())
            {
                auto curl_context_ptr = std::make_unique<curl_context>(*el);
                cc                    = curl_context_ptr.release();
            }
            else
            {
                cc = el->m_curl_context_ready.front().release();
                el->m_curl_context_ready.pop_front();
            }

            cc->init(&el->m_uv_loop, socket);
            curl_multi_assign(el->m_cmh, socket, static_cast<void*>(cc));
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
                curl_multi_assign(el->m_cmh, socket, nullptr);
            }
            break;
        default:
            break;
    }

    return 0;
}

auto uv_close_callback(uv_handle_t * /*handle*/) -> void
{
    // auto* el = static_cast<event_loop*>(handle->data);
    // (void)el;

    // Currently nothing needs to be done since all handles the event loop uses
    // are allocated within the lift::event_loop object and not separate on the heap.
}

auto on_uv_timeout_callback(uv_timer_t* handle) -> void
{
    auto* el = static_cast<event_loop*>(handle->data);
    el->check_actions();
}

auto on_uv_curl_perform_callback(uv_poll_t* req, int status, int events) -> void
{
    auto* cc = static_cast<curl_context*>(req->data);
    auto& el = cc->lift_event_loop();

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

    el.check_actions(cc->curl_sock_fd(), action);
}

auto on_uv_requests_accept_async(uv_async_t* handle) -> void
{
    auto* el = static_cast<event_loop*>(handle->data);

    /**
     * This lock must not have any "curl_*" functions called
     * while it is held, curl has its own internal locks and
     * it can cause a deadlock.  This means we intentionally swap
     * vectors before working on them so we have exclusive access
     * to the Request objects on the event_loop thread.
     */
    {
        std::lock_guard<std::mutex> guard{el->m_pending_requests_lock};
        // swap so we can release the lock as quickly as possible
        el->m_grabbed_requests.swap(el->m_pending_requests);
    }

    for (auto& request_ptr : el->m_grabbed_requests)
    {
        auto executor_ptr = el->acquire_executor();
        executor_ptr->start_async(std::move(request_ptr), el->m_share_ptr.get());
        executor_ptr->prepare();

        // This must be done before adding to the CURLM* object,
        // if not its possible a very fast request could complete
        // before this gets into the multi-map!
        el->add_timeout(*executor_ptr);

        auto curl_code = curl_multi_add_handle(el->m_cmh, executor_ptr->m_curl_handle);

        if (curl_code != CURLM_OK && curl_code != CURLM_CALL_MULTI_PERFORM)
        {
            /**
             * If curl_multi_add_handle fails then notify the user that the request failed to start
             * immediately.
             */
            el->complete_request_normal(*executor_ptr.get(), executor::convert(CURLcode::CURLE_SEND_ERROR));
        }
        else
        {
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
            el->check_actions();
        }
    }

    el->m_grabbed_requests.clear();
}

auto on_uv_timesup_callback(uv_timer_t* handle) -> void
{
    auto* el      = static_cast<event_loop*>(handle->data);
    auto& timesup = el->m_timeouts;

    if (timesup.empty())
    {
        return;
    }

    auto now = uv_now(&el->m_uv_loop);

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

        el->complete_request_timeout(*exe);
        iter = el->remove_timeout(*exe);
    }
}

} // namespace lift
