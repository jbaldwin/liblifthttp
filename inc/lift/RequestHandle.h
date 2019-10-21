#pragma once

#include "lift/Request.h"

#include <memory>

namespace lift {

class RequestPool;
class EventLoop;

/**
 * This is a proxy object to automatically reclaim finished requests
 * into the RequestPool.  The user simply uses it like a std::unique_ptr
 * by accessing the underlying RequestHandle via the * or -> operators.
 */
class RequestHandle {
    friend class RequestPool;
    friend class Request;
    friend class EventLoop;

public:
    ~RequestHandle();
    RequestHandle(const RequestHandle&) = delete;
    RequestHandle(RequestHandle&& from) = default;
    auto operator=(const RequestHandle&) = delete;
    auto operator=(RequestHandle &&) -> RequestHandle& = default;

    /**
     * @return Access to the underlying request handle.
     * @{
     */
    auto operator*() -> Request&;
    auto operator*() const -> const Request&;
    auto operator-> () -> Request*;
    auto operator-> () const -> const Request*;
    /** @} */

private:
    RequestHandle(
        RequestPool* request_pool,
        std::unique_ptr<Request> request_handle);

    /// The request pool that owns this request.
    RequestPool* m_request_pool;
    /// The actual underlying request object.
    std::unique_ptr<Request> m_request_handle;

    /// Friend so it can release the m_request_handle appropriately.
    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;
};

} // lift
