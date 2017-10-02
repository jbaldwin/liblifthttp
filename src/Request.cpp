#include "lift/Request.h"
#include "lift/RequestPool.h"

namespace lift
{

Request::Request(
    RequestPool& request_pool,
    std::unique_ptr<RequestHandle> request_handle
)
    :
        m_request_pool(request_pool),
        m_request_handle(std::move(request_handle))
{

}

Request::~Request()
{
    /**
     * Only move the request handle into the pool if this is the 'valid'
     * request object that still owns the data.
     */
    if(m_request_handle)
    {
        m_request_pool.returnRequest(std::move(m_request_handle));
        m_request_handle = nullptr;
    }
}

auto Request::operator->() -> RequestHandle* {
    return m_request_handle.get();
}

} // lift
