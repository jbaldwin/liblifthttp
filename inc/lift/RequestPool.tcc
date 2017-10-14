#include "lift/CurlPool.h"

namespace lift
{

template<typename Rep, typename Period>
auto RequestPool::Produce(
    const std::string& url,
    std::chrono::duration<Rep, Period> timeout
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
                m_curl_pool->Produce(),
                *m_curl_pool
            )
        );

        return Request(this, std::move(request_handle_ptr));
    }
    else
    {
        auto request_handle_ptr = std::move(m_requests.back());
        m_requests.pop_back();
        m_lock.unlock();

        request_handle_ptr->SetUrl(url);
        request_handle_ptr->SetTimeout(timeout);

        return Request(this, std::move(request_handle_ptr));
    }
}

} // lift
