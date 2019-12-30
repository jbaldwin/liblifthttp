#include "lift/RequestHandle.hpp"
#include "lift/RequestPool.hpp"

namespace lift {

RequestHandle::RequestHandle(
    RequestPool* request_pool,
    std::unique_ptr<Request> request_ptr)
    : m_request_pool(request_pool)
    , m_request_ptr(std::move(request_ptr))
{
}

RequestHandle::~RequestHandle()
{
    /**
     * Only move the request handle into the pool if this is the 'valid'
     * request handle object that still owns the data.
     */
    if (m_request_ptr != nullptr && m_request_pool != nullptr) {
        m_request_pool->returnRequest(std::move(m_request_ptr));
        m_request_pool = nullptr;
        m_request_ptr = nullptr;
    }
}

auto RequestHandle::operator*() -> Request&
{
    return *m_request_ptr;
}

auto RequestHandle::operator*() const -> const Request&
{
    return *m_request_ptr;
}

auto RequestHandle::operator-> () -> Request*
{
    return m_request_ptr.get();
}

auto RequestHandle::operator-> () const -> const Request*
{
    return m_request_ptr.get();
}

} // lift
