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

class event_loop
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
        std::optional<std::vector<ResolveHost>> resolve_hosts{std::nullopt};
        /// Should separate event loops share connection information?
        SharePtr share_ptr{nullptr};
        /// If this functor is provided it is called on the background
        /// thread starting and thread stopping.  This can be used to set the
        /// thread's priority/niceness or possibly changes its thread name.
        on_thread_callback_type on_thread_callback{nullptr};
    };

    /**
     * Creates a new lift event loop to execute many asynchronous HTTP requests simultaneously.
     * @param options See event_loop::options for various options.
     */
    explicit event_loop(
        options opts = options{
            std::nullopt, // reserve connections
            std::nullopt, // max connections
            std::nullopt, // connect timeout
            std::nullopt, // resolve hosts
            nullptr,      // share ptr
            nullptr       // on thread callback
        });

    ~event_loop();

    event_loop(const event_loop&) = delete;
    event_loop(event_loop&&)      = delete;
    auto operator=(const event_loop&) noexcept -> event_loop& = delete;
    auto operator=(event_loop&&) noexcept -> event_loop& = delete;

    /**
     * @return True if the event loop is currently running.
     */
    [[nodiscard]] auto is_running() -> bool { return m_is_running.load(std::memory_order_acquire); }

    /**
     * Stops the event loop from accepting new requests.  It will continue to process
     * existing requests until they are completed.  Note that the ~event_loop() will
     * 'block' until all requests flush as the background even loop process thread
     * won't exit until they are all completed/timed-out/error'ed/etc.
     */
    auto stop() -> void { m_is_stopping.exchange(true, std::memory_order_release); }

    /**
     * @return Gets the number of active HTTP requests currently running.  This includes
     *         the number of pending requests that haven't been started yet (if any).
     */
    [[nodiscard]] auto size() const -> uint64_t { return m_active_request_count.load(std::memory_order_relaxed); }

    /**
     * @return True if there are no requests pending or executing.
     */
    [[nodiscard]] auto empty() const -> bool { return size() == 0; }

    /**
     * Adds a request to process.  The ownership of the request is trasferred into
     * the event loop during execution and returned to the client in the
     * OnCompletHandlerType callback.
     *
     * This function is thread safe.
     *
     * @param request_ptr The request to process.  This request
     *                    will have its OnComplete() handler called
     *                    when its completed/error'ed/etc.
     */
    auto start_request(RequestPtr request_ptr) -> bool;

    /**
     * Adds a batch of requests to process.  The requests in the container will be moved
     * out of the container and into the event_loop, ownership of the requests is transferred
     * into th event loop during execution.
     *
     * This function is thread safe.
     *
     * @tparam container_type A container of class lift::request_ptr.
     * @param requests The batch of requests to process.
     */
    template<typename container_type>
    auto start_requests(container_type requests) -> bool;

private:
    /// Set to true if the event_loop is currently running.
    std::atomic<bool> m_is_running{false};
    /// Set to true if the event_loop is currently shutting down.
    std::atomic<bool> m_is_stopping{false};
    /// The active number of requests running.
    std::atomic<uint64_t> m_active_request_count{0};

    /// The UV event loop to drive libcurl.
    uv_loop_t m_uv_loop{};
    /// The async trigger for injecting new requests into the event loop.
    uv_async_t m_uv_async{};
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
     * Before the event_loop begins working on the pending requests, it swaps
     * the pending requests vector into the grabbed requests vector -- this is done
     * because the pending requests lock could deadlock with internal curl locks!
     */
    std::vector<RequestPtr> m_pending_requests{};
    /// Only accessible from within the event_loop thread.
    std::vector<RequestPtr> m_grabbed_requests{};

    /// The background thread spawned to drive the event loop.
    std::thread m_background_thread{};

    /// List of curl_context objects to re-use for requests, cannot be initialized here due to curl_context being
    /// private.
    std::deque<curl_context_ptr> m_curl_context_ready;

    /// Pool of executors for running requests.
    std::deque<std::unique_ptr<executor>> m_executors{};

    /// The set of resolve hosts to apply to all requests in this event loop.
    std::vector<lift::ResolveHost> m_resolve_hosts{};

    /// When connection time is enabled on an event loop the curl timeout is the longer
    /// timeout value and these timeouts are the shorter value.
    std::multimap<time_point, executor*> m_timeouts{};

    /// If the event loop is provided a Share object then connection information like
    /// DNS/SSL/Data pipelining can be shared across event loops.
    SharePtr m_share_ptr{nullptr};

    /// Functor to call on background thread start/stop.
    on_thread_callback_type m_on_thread_callback{nullptr};

    /// The background thread runs from this function.
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
     * than the Request->OnComplete() function directly.
     * @param exe The request handle to complete.
     * @param status The status of the request when completing.
     */
    auto complete_request_normal(executor& exe, LiftStatus status) -> void;

    /**
     * Completes a request that has timed out but still has connection time remaining.
     * @param exe The request to timeout.
     */
    auto complete_request_timeout(executor& exe) -> void;

    /**
     * Adds the request with the appropriate timeout.
     * If only a timeout exists, the timeout is set directly on CURLM.
     * If timeout + connection time exists AND connectoin time > timeout
     *      timeout is set on the event_loop
     *      connection time is set on Curl
     * If timeout > connection time
     *      timeout is set on the event_loop
     * If no timeout exists nothing is set (infinite -- and could hang).
     */
    auto add_timeout(executor& exe) -> void;

    /**
     * Removes the timeout from the event_loop timer information.
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
     * This function is a friend so it can access the event_loop's m_timeout_timer object and
     * call check_actions().
     *
     * @param cmh The curl multi handle to apply the new timeout for.
     * @param timeout_ms The timeout duration in milliseconds.
     * @param user_data This is a pointer to this event_loop object.
     */
    friend auto curl_start_timeout(CURLM* cmh, long timeout_ms, void* user_data) -> void;

    /**
     * This function is called by libcurl to handle socket actions and update each sockets
     * state within the event_loop.
     *
     * This function is a friend so it can access various event_loop members to drive libcurl
     * and libuv.
     *
     * @param curl The curl handle to handle its socket actions.
     * @param socket The raw socket being handled.
     * @param action The action to apply to the socket (e.g. POLL IN|OUT)
     * @param user_data This is a pointer to this event_loop object.
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
     * This function is a friend so it can access the event_loop flags to assist shutdown.
     * Each handle.data pointer is set to this event_loop object.
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
     * the event_loop.
     *
     * @param handle The async object trigger, this will always be m_async.
     */
    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

    friend auto on_uv_timesup_callback(uv_timer_t* handle) -> void;
};

template<typename container_type>
auto event_loop::start_requests(container_type requests) -> bool
{
    if (m_is_stopping.load(std::memory_order_acquire))
    {
        return false;
    }

    m_active_request_count.fetch_add(std::size(requests), std::memory_order_relaxed);

    // Lock scope
    {
        std::lock_guard<std::mutex> guard(m_pending_requests_lock);
        for (auto& request_ptr : requests)
        {
            if (request_ptr == nullptr)
            {
                continue;
            }
            m_pending_requests.emplace_back(std::move(request_ptr));
        }
    }

    // Notify the even loop thread that there are requests waiting to be picked up.
    uv_async_send(&m_uv_async);

    return true;
}

} // namespace lift
