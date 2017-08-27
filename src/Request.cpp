#include "Request.h"

#include <experimental/string_view>

using std::experimental::string_view;

auto write_data(void* ptr, size_t size, size_t nmemb, void* curl_request_ptr) -> size_t;

Request::Request()
    :
        Request("")
{

}

Request::Request(const std::string& url)
    :
        m_error_code(CURLE_OK)
{
    m_curl_handle = curl_easy_init();
    SetUrl(url);

    curl_easy_setopt(m_curl_handle, CURLOPT_PRIVATE, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1l);
}

Request::~Request()
{
    curl_easy_cleanup(m_curl_handle);
}

auto Request::Perform() -> bool {
    m_error_code = curl_easy_perform(m_curl_handle);
    return (m_error_code == CURLE_OK);
}

auto Request::SetUrl(const std::string& url) -> bool {
    if(url.empty())
    {
        return false;
    }

    m_url = url;
    auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_URL, url.c_str());
    return (error_code == CURLE_OK);
}

auto Request::GetUrl() const -> const std::string& {
    return m_url;
}

auto Request::SetTimeoutMilliseconds(uint64_t timeout_ms) -> bool
{
    auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, &timeout_ms);
    return (error_code == CURLE_OK);
}

auto Request::Reset() -> void
{
    curl_easy_reset(m_curl_handle);
    m_response_data.clear();
    m_response_data.str("");
}

auto Request::GetDownloadData() const -> std::string {
    return m_response_data.str();
}

auto Request::GetTotalTimeMilliseconds() const -> uint64_t
{
    double total_time = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_TOTAL_TIME, &total_time);
    return static_cast<uint64_t>(total_time * 1000);
}

auto Request::TimedOut() -> bool {
    return (m_error_code == CURLE_OPERATION_TIMEDOUT);
}

auto write_data(void* ptr, size_t size, size_t nmemb, void* curl_request_ptr) -> size_t
{
    auto* curl_request = static_cast<Request*>(curl_request_ptr);

    string_view data{static_cast<const char*>(ptr), size * nmemb};
    curl_request->m_response_data << data;

    return size * nmemb;
}
