#pragma once

#include "lift/executor.hpp"
#include "lift/request.hpp"
#include "lift/resolve_host.hpp"
#include "lift/share.hpp"

#include <curl/curl.h>
#include <uv.h>

#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace lift
{
class curl_context;
using curl_context_ptr = std::unique_ptr<curl_context>;

class client
{
    friend curl_context;
    friend executor;

public:
    /// libuv uses simple uint64_t values for millisecond steady clocks.
    using time_point = uint64_t;

    /// Functor type for on background thread creation/deletion.
    using on_thread_callback_type = std::function<void()>;

    struct options
    {
        /// The number of connections to prepare (reserve) for execution.
        std::optional<uint64_t> reserve_connections{std::nullopt};
        /// The maximum number of connections this event loop should
        /// hold open at any given time.  If exceeded the oldest connection
        /// not in use will be removed.
        std::optional<uint64_t> max_connections{std::nullopt};
        /// The amount of time new connections are allowed to setup the connection.
        /// This value will be applied to every request that is executed through
        /// this event loop, but if the individual request has a connect timeout
        /// set on it then that value will be used instead.  Note that on individual
        /// requests this value should be less than the total timeout value, but on
        /// the event loop this should be larger than the total timeout, its a special
        /// feature to allow for long tail connects but very short requests once
        /// the keep-alive connection is established.
        std::optional<std::chrono::milliseconds> connect_timeout{std::nullopt};
        /// A set of host:port combinations to bypass DNS resolving.
        std::optional<std::vector<resolve_host>> resolve_hosts{std::nullopt};
        /// Should separate event loops share connection information?
        share_ptr share{nullptr};
        /// If this functor is provided it is called on the background
        /// thread starting and thread stopping.  This can be used to set the
        /// thread's priority/niceness or possibly changes its thread name.
        on_thread_callback_type on_thread_callback{nullptr};
    };

    /**
     * Creates a new lift event loop to execute many asynchronous HTTP requests simultaneously.
     * @param opts See client::options for various options.
     */
    explicit client(
        options opts = options{
            std::nullopt, // reserve connections
            std::nullopt, // max connections
            std::nullopt, // connect timeout
            std::nullopt, // resolve hosts
            nullptr,      // share ptr
            nullptr       // on thread callback
        });

    ~client();

    client(const client&)                             = delete;
    client(client&&)                                  = delete;
    auto operator=(const client&) noexcept -> client& = delete;
    auto operator=(client&&) noexcept -> client&      = delete;

    /**
     * @return True if the event loop is currently running.
     */
    [[nodiscard]] auto is_running() -> bool { return m_is_running.load(std::memory_order_acquire); }

    /**
     * Stops the client's background event loop from accepting new requests.  It will continue to
     * process existing requests until they are completed.  Note that the ~client() will
     * 'block' until all requests flush as the background even loop process thread
     * won't exit until they are all completed/timed-out/error'ed/etc.
     *
     * This function does not block, it only signals the client to stop accepting requests.
     */
    auto stop() -> void { m_is_stopping.exchange(true, std::memory_order_release); }

    /**
     * @return Gets the number of active HTTP requests currently running.  This includes
     *         the number of pending requests that haven't been started yet (if any).
     */
    [[nodiscard]] auto size() const -> std::size_t { return m_active_request_count.load(std::memory_order_acquire); }

    /**
     * @return True if there are no requests pending or executing.
     */
    [[nodiscard]] auto empty() const -> bool { return size() == 0; }

    /**
     * Starts processing the given request.  The ownership of the request is transferred into the
     * client's background event loop thread during execution and is returned to the user when
     * the future is fulfilled.
     *
     * This function is thread safe and can be called from any thread to start processing a request.
     *
     * @throw std::runtime_error If the request_ptr is nullptr.
     * @param request_ptr The request to process.
     * @return A future that will be fulfilled upon the request completing processing.
     */
    [[nodiscard]] auto start_request(request_ptr&& request_ptr) -> request::async_future_type;

    /**
     * Starts processing the given request.  The ownership of the request is transferred into the
     * client's background event loop thread during execution and is returned to the user when the
     * on complete callback handler is invoked.
     *
     * This function is thread safe and can be called from any thread to start processing a request.
     *
     * @throw std::runtime_error If the request_ptr or callback are nullptr.
     * @param request_ptr The request to process.  This request will have its OnComplete() handler
     *                    called when its completed/error'ed/etc.
     */
    auto start_request(request_ptr&& request_ptr, request::async_callback_type callback) -> void;

    /**
     * Starts processing the set of given requests.  The ownership of the requests are transferred
     * into the client's background event loop thread during execution and they are each individually
     * returned to the user when each future is fulfilled.
     *
     * This function is thread safe and can be called from any thread to start processing requests.
     *
     * Any requests that are given as nullptr will be ignored.
     *
     * @tparam container_type A container with a set of class lift::request_ptr.
     * @param requests The batch of requests to process.
     * @return The set of futures for each request that was started.
     */
    template<typename container_type>
    auto start_requests(container_type&& requests) -> std::vector<request::async_future_type>
    {
        std::vector<request::async_future_type> futures{};
        futures.reserve(std::size(requests));

        size_t amount{std::size(requests)};

        // Prep each request's future prior to acquiring the lock.
        for (auto& request_ptr : requests)
        {
            if (request_ptr != nullptr)
            {
                futures.emplace_back(request_ptr->async_future());
            }
            else
            {
                --amount;
            }
        }

        start_requests_common(std::move(requests), amount);
        return futures;
    }

    /**
     * Starts processing the set of given requests.  The ownership of the requests are transferred
     * into the client's background event loop thread during execution and they are each individually
     * returned to the user when their on complete callback is invoked.  Each request submitted via
     * this function has the same callback used for each request.
     *
     * This function is thread safe and can be called from any thread to start processing requests.
     *
     * Any requests that are given as nullptr will be ignored.
     *
     * @throw std::runtime_error If the callback is nullptr.
     * @tparam container_type A container with a set of class lift::request_ptr.
     * @param requests The batch of requests to process.
     */
    template<typename container_type>
    auto start_requests(container_type&& requests, request::async_callback_type callback) -> void
    {
        if (callback == nullptr)
        {
            throw std::runtime_error{"lift::client::start_requests (callback) The callback cannot be nullptr."};
        }

        size_t amount{std::size(requests)};

        // Prep each request's callback prior to acquiring the lock.
        for (auto& request_ptr : requests)
        {
            if (request_ptr != nullptr)
            {
                request_ptr->async_callback(callback);
            }
            else
            {
                --amount;
            }
        }

        start_requests_common(std::move(requests), amount);
    }

private:
    /// Set to true if the client is currently running.
    std::atomic<bool> m_is_running{false};
    /// Set to true if the client is currently shutting down.
    std::atomic<bool> m_is_stopping{false};
    /// The active number of requests running.
    std::atomic<std::size_t> m_active_request_count{0};

    /// The UV event loop to drive libcurl.
    uv_loop_t m_uv_loop{};
    /// The async trigger for injecting new requests into the event loop.
    uv_async_t m_uv_async{};
    /// The async trigger to let uv_run() know its being shutdown
    uv_async_t m_uv_async_shutdown_pipe{};
    /// libcurl requires a single timer to drive internal timeouts/wake-ups.
    uv_timer_t m_uv_timer_curl{};
    /// If set, the amount of time connections are allowed to connect, this can be
    /// longer than the timeout of the request.
    std::optional<std::chrono::milliseconds> m_connect_timeout{std::nullopt};
    /// Timeout timer.
    uv_timer_t m_uv_timer_timeout{};
    /// The libcurl multi handle for driving multiple easy handles at once.
    CURLM* m_cmh{curl_multi_init()};

    /// Pending requests are safely queued via this lock.
    std::mutex m_pending_requests_lock{};
    /**
     * Pending requests are stored in this vector until they are picked up on the next
     * uv loop iteration.  Any memory accesses to this object should first acquire the
     * m_pending_requests_lock to guarantee thread safety.
     *
     * Before the client begins working on the pending requests, it swaps
     * the pending requests vector into the grabbed requests vector -- this is done
     * because the pending requests lock could deadlock with internal curl locks!
     */
    std::vector<request_ptr> m_pending_requests{};
    /// Only accessible from within the client thread.
    std::vector<request_ptr> m_grabbed_requests{};

    /// The background thread spawned to drive the event loop.
    std::thread m_background_thread{};

    /// List of curl_context objects to re-use for requests, cannot be initialized here due to curl_context being
    /// private.
    std::deque<curl_context_ptr> m_curl_context_ready;

    /// Pool of executors for running requests.
    std::deque<std::unique_ptr<executor>> m_executors{};

    /// The set of resolve hosts to apply to all requests in this event loop.
    std::vector<lift::resolve_host> m_resolve_hosts{};

    /// When connection time is enabled on an event loop the curl timeout is the longer
    /// timeout value and these timeouts are the shorter value.
    std::multimap<time_point, executor*> m_timeouts{};

    /// If the event loop is provided a share object then connection information like
    /// DNS/SSL/Data pipelining can be shared across event loops.
    share_ptr m_share_ptr{nullptr};

    /// Functor to call on background thread start/stop.
    on_thread_callback_type m_on_thread_callback{nullptr};

    /**
     * Common code between future and callback start request functions.
     */
    auto start_request_common(request_ptr&& request_ptr) -> void;

    /**
     * Common code between future and callback start requests functions.
     */
    template<typename container_type>
    auto start_requests_common(container_type&& requests, size_t amount) -> void
    {
        // Whoops, this client is actually shutting down.
        if (m_is_stopping.load(std::memory_order_acquire))
        {
            for (auto& request_ptr : requests)
            {
                start_request_notify_failed_start(std::move(request_ptr));
            }
            return;
        }

        m_active_request_count.fetch_add(amount, std::memory_order_release);

        {
            std::scoped_lock<std::mutex> lk{m_pending_requests_lock};
            m_pending_requests.reserve(m_pending_requests.size() + amount);
            for (auto& request_ptr : requests)
            {
                if (request_ptr != nullptr)
                {
                    m_pending_requests.emplace_back(std::move(request_ptr));
                }
            }
        }

        // Notify the event loop thread that there are requests waiting to be picked up.
        uv_async_send(&m_uv_async);
    }

    /**
     * Utility function to notify the user correctly when a request fails to start.
     */
    static auto start_request_notify_failed_start(request_ptr&& request_ptr) -> void
    {
        response r{};

        r.m_lift_status = lift_status::error_failed_to_start;

        // This http status code isn't perfect, but its better than nothing I think?
        r.m_status_code   = lift::http::status_code::http_500_internal_server_error;
        r.m_total_time    = 0;
        r.m_num_connects  = 0;
        r.m_num_redirects = 0;

        auto& on_complete_handler = request_ptr->m_on_complete_handler.m_object.value();

        if (std::holds_alternative<request::async_callback_type>(on_complete_handler))
        {
            auto& callback = std::get<request::async_callback_type>(on_complete_handler);
            callback(std::move(request_ptr), std::move(r));
        }
        else if (std::holds_alternative<request::async_promise_type>(on_complete_handler))
        {
            auto& promise = std::get<request::async_promise_type>(on_complete_handler);
            promise.set_value(std::make_pair(std::move(request_ptr), std::move(r)));
        }
        // else do nothing for std::monostate, no way to actually report the client is shutting down.
    }

    /*
     * The background event loop thread runs from this function.
     */
    auto run() -> void;

    /**
     * Checks current pending curl actions like timeouts.
     */
    auto check_actions() -> void;

    /**
     * Checks current pending curl actions for a specific socket/action (event_bitmask)
     * @param socket The socket to check current actions on.
     * @param event_bitmask The type of action (IN|OUT|INOUT|ERR).
     */
    auto check_actions(curl_socket_t socket, int event_bitmask) -> void;

    /**
     * Completes a request to pass ownership back to the user land.
     * Manages internal state accordingly, always call this function rather
     * than the request->OnComplete() function directly.
     * @param exe_ptr The request handle to complete.
     * @param status The status of the request when completing.
     */
    auto complete_request_normal(executor_ptr exe_ptr, lift_status status) -> void;

    auto complete_request_normal_common(executor& exe, lift_status status) -> void;

    /**
     * Completes a request that has timed out but still has connection time remaining.
     * @param exe The request to timeout.
     */
    auto complete_request_timeout(executor& exe) -> void;
    auto complete_request_timeout_common(executor& exe) -> request_ptr;

    /**
     * Adds the request with the appropriate timeout.
     * If only a timeout exists, the timeout is set directly on CURLM.
     * If timeout + connection time exists AND connectoin time > timeout
     *      timeout is set on the client
     *      connection time is set on Curl
     * If timeout > connection time
     *      timeout is set on the client
     * If no timeout exists nothing is set (infinite -- and could hang).
     */
    auto add_timeout(executor& exe) -> void;

    /**
     * Removes the timeout from the client timer information.
     * Connection time can still fire from curl but the request's
     * on complete handler won't be called.
     */
    auto remove_timeout(executor& exe) -> std::multimap<uint64_t, executor*>::iterator;

    /**
     * Updates the event loop timeout information.
     */
    auto update_timeouts() -> void;

    auto acquire_executor() -> std::unique_ptr<executor>;
    auto return_executor(std::unique_ptr<executor> executor_ptr) -> void;

    /**
     * This function is called by libcurl to start a timeout with duration timeout_ms.
     *
     * This function is a friend so it can access the client's m_timeout_timer object and
     * call check_actions().
     *
     * @param cmh The curl multi handle to apply the new timeout for.
     * @param timeout_ms The timeout duration in milliseconds.
     * @param user_data This is a pointer to this client object.
     */
    friend auto curl_start_timeout(CURLM* cmh, long timeout_ms, void* user_data) -> void;

    /**
     * This function is called by libcurl to handle socket actions and update each sockets
     * state within the client.
     *
     * This function is a friend so it can access various client members to drive libcurl
     * and libuv.
     *
     * @param curl The curl handle to handle its socket actions.
     * @param socket The raw socket being handled.
     * @param action The action to apply to the socket (e.g. POLL IN|OUT)
     * @param user_data This is a pointer to this client object.
     * @param socketp A pointer to the curl_context if this is an existing request.  This will
     *                be a nullptr if this is a new request and will then be created.
     * @return Always returns zero.
     */
    friend auto curl_handle_socket_actions(CURL* curl, curl_socket_t socket, int action, void* user_data, void* socketp)
        -> int;

    /**
     * This function is called by uv_close() to say the handles resources are properly closed.
     * This is used to make sure the m_timeout_timer and m_async handles are stopped and
     * the event loop can be closed.
     *
     * This function is a friend so it can access the client flags to assist shutdown.
     * Each handle.data pointer is set to this client object.
     *
     * @param handle The handle that is being closed.
     */
    friend auto uv_close_callback(uv_handle_t* handle) -> void;

    /**
     * This function is called by libuv when the m_timeout_timer expires.
     *
     * This function is a friend so it can call check_actions() and have libcurl timeout
     * jobs that have expired.
     *
     * @param handle The timer object trigger, this will always be m_timeout_timer.
     */
    friend auto on_uv_timeout_callback(uv_timer_t* handle) -> void;

    /**
     * This function is called by libuv to call check_actions(socket, action) for a specific
     * socket and action (POLL IN|OUT).
     * @param req Libuv's context around the 'pollable' item, in this case the curl handle.
     * @param status The status of the poll item, a non-zero value implies an error.  A zero value
     *               means the 'event's parameter will denote what to register this poll item for,
     *               e.g. IN|OUT.
     * @param events The poll items to register for, e.g. IN|OUT.
     */
    friend auto on_uv_curl_perform_callback(uv_poll_t* req, int status, int events) -> void;

    /**
     * This function is called by libuv when the m_async is triggered with a new request.
     *
     * This function is a friend so it can pull pending requests and inject them into
     * the client.
     *
     * @param handle The async object trigger, this will always be m_async.
     */
    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

    friend auto on_uv_shutdown_async(uv_async_t* handle) -> void;

    friend auto on_uv_timesup_callback(uv_timer_t* handle) -> void;
};

} // namespace lift
