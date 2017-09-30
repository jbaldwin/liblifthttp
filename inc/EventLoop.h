#pragma once

#include "AsyncRequest.h"
#include "IRequestCallbacks.h"

#include <curl/curl.h>
#include <uv.h>

#include <vector>
#include <mutex>
#include <memory>
#include <list>

class EventLoop
{
public:
    /**
     * Creates a new evenloop with the given request callbacks.
     * @param request_callbacks Callbacks used for when requests
     *                          complete/error/timeout.  The EventLoop
     *                          owns the lifetime of this callback object
     *                          and will release it upon destruction.
     */
    explicit EventLoop(
        std::unique_ptr<IRequestCallbacks> request_callbacks
    );

    ~EventLoop();

    /**
     * No copying allowed.
     */
    EventLoop(const EventLoop& copy) = delete;

    /**
     * Moving is allowed.
     */
    EventLoop(EventLoop&& move) = default;

    /**
     * No copy assignment allowed.
     */
    auto operator=(const EventLoop& assign) = delete;

    /**
     * Move assign allowed.
     */
    auto operator=(EventLoop&& assign) -> EventLoop& = default;

    /**
     * Runs the event loop until EventLoop::Stop() is called.
     */
    auto Run() -> void;

    /**
     * Runs a single iteration of the EventLoop and the returns
     * control to the caller.
     */
    auto RunOnce() -> void;

    /**
     * Stops the EventLoop and shutsdown all resources.
     */
    auto Stop() -> void;

    /**
     * Adds a request to process.  This function is safe to call from
     * the same or different threads.
     * @param async_request_ptr The request to process.  This request
     *                          will have the IRequestCallbacks called
     *                          when this request completes/timesout/errors.
     */
    auto AddRequest(
        std::unique_ptr<AsyncRequest> async_request_ptr
    ) -> void;

    /**
     * @return Gets a borrowed reference to the callback functions.
     * @{
     */
    auto GetRequestCallbacks() -> IRequestCallbacks&;
    auto GetRequestCallbacks() const -> const IRequestCallbacks&;
    /** @} */

private:
    std::unique_ptr<IRequestCallbacks> m_request_callbacks; ///< Callback functions

    uv_loop_t* m_loop; ///< The UV event loop to drive libcurl.
    uv_async_t m_async; ///< An async trigger for injecting new requests into the event loop.
    uv_timer_t m_timeout_timer; ///< libcurl requires a single timer to drive timeouts/wake-ups.
    CURLM* m_cmh; ///< The libcurl multi handle for driving multiple easy handles at once.

    std::mutex m_pending_requests_lock; ///< Pending requests are safely queued via this lock.
    /**
     * Pending requests are stored in this vector until they are picked up on the next
     * uv loop iteration.  Any memory accesses to this object should first acquire the
     * m_pending_requests_lock to guarantee thread safety.
     */
    std::vector<std::unique_ptr<AsyncRequest>> m_pending_requests;

    /**
     * Active requests that are being processed.  The data structure used is a list for quick
     * addition of new requests and also quick removal of requests that finish asynchronously.
     */
    std::list<std::unique_ptr<AsyncRequest>> m_active_requests;

    bool m_async_closed; ///< Flag to denote that the m_async handle has been closed on shutdown.
    bool m_timeout_timer_closed; ///< Flag to denote that the m_timeout_timer has been closed on shutdown.

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
        int event_bitmask
    ) -> void;

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
        void* user_data
    ) -> void;

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
        void* socketp
    ) -> int;

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
        uv_handle_t* handle
    ) -> void;

    /**
     * This function is called by libuv when the m_timeout_timer expires.
     *
     * This function is a friend so it can call checkActions() and have libcurl timeout
     * jobs that have expired.
     *
     * @param handle The timer object trigger, this will always be m_timeout_timer.
     * @param status Unused.
     */
    friend auto on_uv_timeout_callback(
        uv_timer_t* handle,
        int status
    ) -> void;

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
        int events
    ) -> void;

    /**
     * This function is called by libuv when the m_async is triggered with a new request.
     *
     * This function is a friend so it can pull pending requests and inject them into
     * the EventLoop.
     *
     * @param async The async object trigger, this will always be m_async.
     * @param status Unused.
     */
    friend auto requests_accept_async(
        uv_async_t* async,
        int status
    ) -> void;
};
