#include "CurlPool.h"

namespace lift
{

CurlPool::~CurlPool()
{
    for(auto* curl_handle : m_curl_handles)
    {
        curl_easy_cleanup(curl_handle);
    }
    m_curl_handles.clear();
}

auto CurlPool::Produce() -> CURL*
{
    m_lock.lock();
    if(m_curl_handles.empty())
    {
        // Intentionally unlocking as the curl_easy_init() function is very expensive.
        m_lock.unlock();
        return curl_easy_init();
    }
    else
    {
        auto* curl_handle = m_curl_handles.back();
        m_curl_handles.pop_back();
        m_lock.unlock();
        return curl_handle;
    }
}

auto CurlPool::Return(
    CURL* curl_handle
) -> void
{
    std::lock_guard<std::mutex> guard(m_lock);
    m_curl_handles.emplace_back(curl_handle);
}

} // lift
