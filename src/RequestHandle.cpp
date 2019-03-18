#include "lift/RequestHandle.h"
#include "lift/RequestPool.h"

#include "lift/EventLoop.h"

namespace lift {

RequestHandle::RequestHandle(
    RequestPool* request_pool,
    std::unique_ptr<Request> request_handle)
    : m_request_pool(request_pool)
    , m_request_handle(std::move(request_handle))
{
}

RequestHandle::~RequestHandle()
{
    /**
     * Only move the request handle into the pool if this is the 'valid'
     * request object that still owns the data.
     */
    if (m_request_handle != nullptr && m_request_pool != nullptr) {
        m_request_pool->returnRequest(std::move(m_request_handle));
        m_request_pool = nullptr;
        m_request_handle = nullptr;
    }
}

auto RequestHandle::operator*() -> Request&
{
    return *m_request_handle;
}

auto RequestHandle::operator*() const -> const Request&
{
    return *m_request_handle;
}

auto RequestHandle::operator-> () -> Request*
{
    return m_request_handle.get();
}

auto RequestHandle::operator-> () const -> const Request*
{
    return m_request_handle.get();
}

} // lift
