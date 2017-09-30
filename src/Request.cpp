#include "Request.h"

namespace lift
{

auto curl_write_data(
    void* ptr,
    size_t size,
    size_t nmemb,
    void* user_ptr
) -> size_t;

Request::Request()
    :
        Request("")
{

}

Request::Request(
    const std::string& url,
    uint64_t timeout_ms
)
    :
        m_status_code(RequestStatus::SUCCESS)
{
    m_curl_handle = curl_easy_init(); // TODO create pool for curl handles
    SetUrl(url);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    curl_easy_setopt(m_curl_handle, CURLOPT_PRIVATE,        this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION,  curl_write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA,      this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL,       1l);
    curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1l);
#pragma clang diagnostic pop

    SetTimeoutMilliseconds(timeout_ms);

    // TODO make the reservation configurable.
    m_response_data.reserve(16'384);
}

Request::~Request()
{
    curl_easy_cleanup(m_curl_handle);
}

auto Request::Perform() -> bool {
    auto curl_error_code = curl_easy_perform(m_curl_handle);
    m_status_code = curl_code2request_status(curl_error_code);
    return (m_status_code == RequestStatus::SUCCESS);
}

auto Request::SetUrl(const std::string& url) -> bool {
    if(url.empty())
    {
        return false;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_URL, url.c_str());
    if(error_code == CURLE_OK)
    {
        char* curl_url = nullptr;
        curl_easy_getinfo(m_curl_handle, CURLINFO_EFFECTIVE_URL, &curl_url);
        if(curl_url)
        {
            m_url = string_view(curl_url, std::strlen(curl_url));
            return true;
        }
    }
#pragma clang diagnostic pop
    return false;
}

auto Request::GetUrl() const -> string_view {
    return m_url;
}

auto Request::SetTimeoutMilliseconds(uint64_t timeout_ms) -> bool
{
    if(timeout_ms > 0)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
        auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_ms));
#pragma clang diagnostic pop
        return (error_code == CURLE_OK);
    }
    return false;
}

auto Request::SetFollowRedirects(
    bool follow_redirects
) -> bool
{
    long curl_value = (follow_redirects) ? 1L : 0L;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, curl_value);
#pragma clang diagnostic pop
    return (error_code == CURLE_OK);
}

auto Request::Reset() -> void
{
    m_url = string_view();
    curl_easy_reset(m_curl_handle);
    m_status_code = RequestStatus::SUCCESS;
    m_response_data.clear();
}

auto Request::GetDownloadData() const -> const std::string&
{
    return m_response_data;
}

auto Request::GetTotalTimeMilliseconds() const -> uint64_t
{
    double total_time = 0;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    curl_easy_getinfo(m_curl_handle, CURLINFO_TOTAL_TIME, &total_time);
#pragma clang diagnostic pop
    return static_cast<uint64_t>(total_time * 1000);
}

auto Request::HasError() const -> bool
{
    return m_status_code != RequestStatus::SUCCESS;
}

auto Request::GetStatus() const -> RequestStatus
{
    return m_status_code;
}

auto Request::curl_code2request_status(
    CURLcode curl_code
) -> RequestStatus
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(curl_code)
    {
        case CURLcode::CURLE_OK:
            return RequestStatus::SUCCESS;
        case CURLcode::CURLE_GOT_NOTHING:
            return RequestStatus::RESPONSE_EMPTY;
        case CURLcode::CURLE_OPERATION_TIMEDOUT:
            return RequestStatus::TIMEOUT;
        case CURLcode::CURLE_COULDNT_CONNECT:
            return RequestStatus::CONNECT_ERROR;
        case CURLcode::CURLE_COULDNT_RESOLVE_HOST:
            return RequestStatus::CONNECT_DNS_ERROR;
        case CURLcode::CURLE_SSL_CONNECT_ERROR:
            return RequestStatus::CONNECT_SSL_ERROR;
        default:
            return RequestStatus::ERROR;
    }
#pragma clang diagnostic pop
}

auto curl_write_data(
    void* ptr,
    size_t size,
    size_t nmemb,
    void* user_ptr
) -> size_t
{
    auto* raw_request_ptr = static_cast<Request*>(user_ptr);
    size_t data_length = size * nmemb;
    raw_request_ptr->m_response_data.append(static_cast<const char*>(ptr), data_length);
    return data_length;
}

} // lift
