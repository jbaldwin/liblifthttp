#include "lift/RequestPool.h"
#include "lift/CurlPool.h"

namespace lift
{

RequestPool::RequestPool()
    : m_lock(),
      m_requests(),
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
    const std::string& url
) -> Request
{
    using namespace std::chrono_literals;
    return Produce(url, nullptr, 0ms);
}

auto RequestPool::Produce(
    const std::string& url,
    std::chrono::milliseconds timeout
) -> Request {
    return Produce(url, nullptr, timeout);
}

auto RequestPool::Produce(
    const std::string& url,
    OnCompleteHandler on_complete_handler,
    std::chrono::milliseconds timeout
) -> Request
{
    m_lock.lock();
    if(m_requests.empty())
    {
        m_lock.unlock();

        // Cannot use std::make_unique here since RequestHandle ctor is private friend.
        auto request_handle_ptr = std::unique_ptr<RequestHandle>(
            new RequestHandle(
                url,
                timeout,
                *this,
                m_curl_pool->Produce(),
                *m_curl_pool,
                on_complete_handler
            )
        );

        return Request(this, std::move(request_handle_ptr));
    }
    else
    {
        auto request_handle_ptr = std::move(m_requests.back());
        m_requests.pop_back();
        m_lock.unlock();

        request_handle_ptr->SetOnCompleteHandler(on_complete_handler);
        request_handle_ptr->SetUrl(url);
        request_handle_ptr->SetTimeout(timeout);

        return Request(this, std::move(request_handle_ptr));
    }
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
