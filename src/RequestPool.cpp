#include "lift/RequestPool.h"
#include "CurlPool.h"

namespace lift
{

RequestPool::RequestPool()
    :
        m_curl_pool(std::make_unique<CurlPool>())
{

}

RequestPool::~RequestPool()
{
    /**
     * Clearing requests is important so that they are all cleaned up
     * before the CurlPool destructs since the Request destructor
     * will call CurlPool::Return(curl_handle).
     */
    m_requests.clear();
}

auto RequestPool::Produce(
    const std::string& url,
    uint64_t timeout_ms
) -> Request
{
    m_lock.lock();
    if(m_requests.empty())
    {
        m_lock.unlock();
        auto request_handle_ptr = std::unique_ptr<RequestHandle>(
            new RequestHandle(url, timeout_ms, m_curl_pool->Produce(), *m_curl_pool)
        );

        return Request(*this, std::move(request_handle_ptr));
    }
    else
    {
        auto request_handle_ptr = std::move(m_requests.back());
        m_requests.pop_back();
        m_lock.unlock();

        request_handle_ptr->SetUrl(url);
        request_handle_ptr->SetTimeoutMilliseconds(timeout_ms);

        return Request(*this, std::move(request_handle_ptr));
    }
}

auto RequestPool::returnRequest(
    std::unique_ptr<RequestHandle> request
) -> void
{
    std::lock_guard<std::mutex> guard(m_lock);
    m_requests.emplace_back(std::move(request));
}

} // lift
