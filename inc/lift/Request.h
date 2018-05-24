#pragma once

#include "lift/RequestHandle.h"

#include <memory>

namespace lift
{

class RequestPool;
class EventLoop;

/**
 * This is a proxy object to automatically reclaim finished requests
 * into the RequestPool.  The user simply uses it like a std::unique_ptr
 * by accessing the underlying RequestHandle via the * or -> operators.
 */
class Request
{
    friend class RequestPool;
    friend class RequestHandle;
    friend class EventLoop;
public:

    ~Request();
    Request(const Request&) = delete;                   ///< No copying
    Request(Request&& from) = default;                  ///< Can move
    auto operator = (const Request&) = delete;          ///< No copy assign
    auto operator = (Request&&) -> Request& = default;  ///< Can move assign

    /**
     * @return Access to the underlying request handle.
     * @{
     */
    auto operator * () -> RequestHandle&;
    auto operator * () const -> const RequestHandle&;
    auto operator -> () -> RequestHandle*;
    auto operator -> () const -> const RequestHandle*;
    /** @} */

private:
    Request(
        RequestPool* request_pool,
        std::unique_ptr<RequestHandle> request_handle
    );

    RequestPool* m_request_pool;                        ///< The request pool that owns this request.
    std::unique_ptr<RequestHandle> m_request_handle;    ///< The actual underlying request object.

    friend auto requests_accept_async(
        uv_async_t* async
    ) -> void; ///< Friend so it can release the m_request_handle appropriately.
};

} // lift
