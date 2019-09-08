#pragma once

#include "lift/RequestPool.h"

#include <curl/curl.h>
#include <uv.h>

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace lift {

class CurlContext;

class EventLoop {
    friend CurlContext;

public:
    /**
     * Creates a new lift event loop.
     */
    EventLoop();

    /**
     * Stops the EventLoop and shuts down all resources.
     */
    ~EventLoop();

    EventLoop(const EventLoop& copy) = delete;
    EventLoop(EventLoop&& move) = delete;
    auto operator=(const EventLoop& copy_assign) -> EventLoop& = delete;
    auto operator=(EventLoop&& move_assign) -> EventLoop& = delete;

    /**
     * @return True if the event loop is currently running.
     */
    auto IsRunning() -> bool;

    /**
     * Stops the event loop from accepting new requests.  It will continue to process
     * existing requests.
     */
    auto Stop() -> void;

    /**
     * @return Gets the number of active HTTP requests currently running.
     */
    [[nodiscard]]
    auto GetActiveRequestCount() const -> uint64_t;

    /**
     * @return The request pool for this EventLoop.
     */
    auto GetRequestPool() -> RequestPool&;

    /**
     * Adds a request to process.
     *
     * This function is thread safe.
     *
     * @param request The request to process.  This request
     *                will have the IRequestCb called
     *                when this request completes/timesout/errors.
     */
    auto StartRequest(
        RequestHandle request) -> bool;

    /**
     * Adds a batch of requests to process.  The requests in the container will be moved
     * out of the container and into the EventLoop.
     *
     * This function is thread safe.
     *
     * @tparam Container A container of class Request.
     * @param requests The batch of requests to process.
     */
    template <typename Container>
    auto StartRequests(
        Container requests) -> void;

private:
    /**
     * Each event loop gets its own private request pool for efficiency.
     * This needs to be first so it de-allocates all its RequestHandles on shutdown.
     */
    RequestPool m_request_pool {};

    /// Set to true if the EventLoop is currently running.
    std::atomic<bool> m_is_running { false };
    /// Set to true if the EventLoop is currently shutting down.
    std::atomic<bool> m_is_stopping { false };
    /// The active number of requests running.
    std::atomic<uint64_t> m_active_request_count { 0 };

    /// The UV event loop to drive libcurl.
    uv_loop_t* m_loop { uv_loop_new() };
    /// The async trigger for injecting new requests into the event loop.
    uv_async_t m_async {};
    /// libcurl requires a single timer to drive timeouts/wake-ups.
    uv_timer_t m_timeout_timer {};
    /// The libcurl multi handle for driving multiple easy handles at once.
    CURLM* m_cmh { curl_multi_init() };

    /// Pending requests are safely queued via this lock.
    std::mutex m_pending_requests_lock {};
    /**
     * Pending requests are stored in this vector until they are picked up on the next
     * uv loop iteration.  Any memory accesses to this object should first acquire the
     * m_pending_requests_lock to guarantee thread safety.
     *
     * Before the EventLoop begins working on the pending requests, it swaps
     * the pending requests vector into the grabbed requests vector -- this is done
     * because the pending requests lock could deadlock with internal curl locks!
     */
    std::vector<RequestHandle> m_pending_requests {};
    /// Only accessible from within the EventLoop thread.
    std::vector<RequestHandle> m_grabbed_requests {};

    /// The background thread spawned to drive the event loop.
    std::thread m_background_thread {};

    /// List of CurlContext objects to re-use for requests, cannot be initialized here due to CurlContext being private.
    std::deque<std::unique_ptr<CurlContext>> m_curl_context_ready{};

    /// Flag to denote that the m_async handle has been closed on shutdown.
    std::atomic<bool> m_async_closed { false };
    /// Flag to denote that the m_timeout_timer has been closed on shutdown.
    std::atomic<bool> m_timeout_timer_closed { false };

    /// The background thread runs from this function.
    auto run() -> void;

    /**
     * Checks current pending curl actions like timeouts.
     */
    auto checkActions() -> void;

    /**
     * Checks current pending curl actions for a specific socket/action (event_bitmask)
     * @param socket The socket to check current actions on.
     * @param event_bitmask The type of action (IN|OUT|ERR).
     */
    auto checkActions(
        curl_socket_t socket,
        int event_bitmask) -> void;

    /**
     * This function is called by libcurl to start a timeout with duration timeout_ms.
     *
     * This function is a friend so it can access the EventLoop's m_timeout_timer object and
     * call checkActions().
     *
     * @param cmh The curl multi handle to apply the new timeout for.
     * @param timeout_ms The timeout duration in milliseconds.
     * @param user_data This is a pointer to this EventLoop object.
     */
    friend auto curl_start_timeout(
        CURLM* cmh,
        long timeout_ms,
        void* user_data) -> void;

    /**
     * This function is called by libcurl to handle socket actions and update each sockets
     * state within the EventLoop.
     *
     * This function is a friend so it can access various EventLoop members to drive libcurl
     * and libuv.
     *
     * @param curl The curl handle to handle its socket actions.
     * @param socket The raw socket being handled.
     * @param action The action to apply to the socket (e.g. POLL IN|OUT)
     * @param user_data This is a pointer to this EventLoop object.
     * @param socketp A pointer to the CurlContext if this is an existing request.  This will
     *                be a nullptr if this is a new request and will then be created.
     * @return Always returns zero.
     */
    friend auto curl_handle_socket_actions(
        CURL* curl,
        curl_socket_t socket,
        int action,
        void* user_data,
        void* socketp) -> int;

    /**
     * This function is called by uv_close() to say the handles resources are properly closed.
     * This is used to make sure the m_timeout_timer and m_async handles are stopped and
     * the event loop can be closed.
     *
     * This function is a friend so it can access the EventLoop flags to assist shutdown.
     * Each handle.data pointer is set to this EventLoop object.
     *
     * @param handle The handle that is being closed.
     */
    friend auto uv_close_callback(
        uv_handle_t* handle) -> void;

    /**
     * This function is called by libuv when the m_timeout_timer expires.
     *
     * This function is a friend so it can call checkActions() and have libcurl timeout
     * jobs that have expired.
     *
     * @param handle The timer object trigger, this will always be m_timeout_timer.
     */
    friend auto on_uv_timeout_callback(
        uv_timer_t* handle) -> void;

    /**
     * This function is called by libuv to call checkActions(socket, action) for a specific
     * socket and action (POLL IN|OUT).
     * @param req Libuv's context around the 'pollable' item, in this case the curl handle.
     * @param status The status of the poll item, a non-zero value implies an error.  A zero value
     *               means the 'event's parameter will denote what to register this poll item for,
     *               e.g. IN|OUT.
     * @param events The poll items to register for, e.g. IN|OUT.
     */
    friend auto on_uv_curl_perform_callback(
        uv_poll_t* req,
        int status,
        int events) -> void;

    /**
     * This function is called by libuv when the m_async is triggered with a new request.
     *
     * This function is a friend so it can pull pending requests and inject them into
     * the EventLoop.
     *
     * @param async The async object trigger, this will always be m_async.
     */
    friend auto requests_accept_async(
        uv_async_t* handle) -> void;
};

} // lift

#include "EventLoop.tcc"
