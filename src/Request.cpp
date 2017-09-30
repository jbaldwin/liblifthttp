#include "lift/Request.h"

namespace lift
{

auto curl_write_header(
    char* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr
) -> size_t;

auto curl_write_data(
    void* buffer,
    size_t size,
    size_t nitems,
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
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, curl_write_header);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA,     this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION,  curl_write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA,      this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL,       1l);
    curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1l);
#pragma clang diagnostic pop

    SetTimeoutMilliseconds(timeout_ms);

    // TODO make the buffer reservations configurable.
    m_response_headers.reserve(4096);
    m_response_headers_idx.reserve(16);
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
            m_url = StringView(curl_url, std::strlen(curl_url));
            return true;
        }
    }
#pragma clang diagnostic pop
    return false;
}

auto Request::GetUrl() const -> StringView {
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
    // Intentionally do not call curl_easy_reset() as it wipes the curl_easy_setopt() settings.

    m_url = StringView();
    m_status_code = RequestStatus::SUCCESS;
    m_response_headers.clear();
    m_response_data.clear();
}

auto Request::GetResponseHeaders() const -> const std::vector<Header>&
{
    return m_response_headers_idx;
}

auto Request::GetResponseData() const -> const std::string&
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

auto curl_write_header(
    char* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr
) -> size_t
{
    auto* raw_request_ptr = static_cast<Request*>(user_ptr);
    size_t data_length = size * nitems;

    StringView data_view(buffer, data_length);

    if(data_view.empty())
    {
        return data_length;
    }

    // Ignore empty header lines from curl.
    if(data_view.length() == 2 && data_view == "\r\n")
    {
        return data_length;
    }
    // Ignore the HTTP/ 'header' line from curl.
    if(data_view.length() >= 4 && data_view.substr(0, 5) == "HTTP/")
    {
        return data_length;
    }

    // Drop the trailing \r\n from the header.
    if(data_view.length() >= 2)
    {
        size_t rm_size = 0;
        if(data_view[data_view.length() - 1] == '\n')
        {
            ++rm_size;
        }
        if(data_view[data_view.length() - 2] == '\r')
        {
            ++rm_size;
        }
        data_view.remove_suffix(rm_size);
    }

    // Append the entire header into the full header buffer.
    raw_request_ptr->m_response_headers.append(data_view.data(), data_view.length());

    // Calculate and append the Header view object.
    const char* start = raw_request_ptr->m_response_headers.c_str();
    auto total_length = raw_request_ptr->m_response_headers.length();
    StringView request_data_view((start + total_length) - data_view.length(), data_view.length());
    raw_request_ptr->m_response_headers_idx.emplace_back(request_data_view);

    return data_length;
}

auto curl_write_data(
    void* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr
) -> size_t
{
    auto* raw_request_ptr = static_cast<Request*>(user_ptr);
    size_t data_length = size * nitems;
    raw_request_ptr->m_response_data.append(static_cast<const char*>(buffer), data_length);
    return data_length;
}

} // lift
