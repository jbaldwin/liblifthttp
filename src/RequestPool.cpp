#include "lift/RequestPool.h"

namespace lift
{

RequestPool::RequestPool()
    : m_curl_pool(std::make_unique<CurlPool>())
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
    const std::string& url
) -> Request
{
    using namespace std::chrono_literals;
    return Produce(url, 0ms);
}

auto RequestPool::returnRequest(
    std::unique_ptr<RequestHandle> request
) -> void
{
    request->Reset(); // Reset the request if it is returned to the pool.
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_requests.emplace_back(std::move(request));
    }
}

} // lift
