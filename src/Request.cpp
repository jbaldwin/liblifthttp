#include "lift/Request.h"
#include "CurlPool.h"

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

Request::Request(
    const std::string& url,
    uint64_t timeout_ms,
    CURL* curl_handle,
    CurlPool& curl_pool
)
    :
        m_curl_handle(curl_handle),
        m_curl_pool(curl_pool),
        m_curl_request_headers(nullptr),
        m_status_code(RequestStatus::BUILDING)
{
    init();
    SetUrl(url);
    SetTimeoutMilliseconds(timeout_ms);
}

Request::~Request()
{
    Reset();
    if(m_curl_handle)
    {
        m_curl_pool.Return(m_curl_handle);
        m_curl_handle = nullptr;
    }
}

auto Request::init() -> void
{
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

    // TODO make the buffer reservations configurable.
    m_request_headers.reserve(16'384);
    m_request_headers_idx.reserve(16);
    m_response_headers.reserve(16'384);
    m_response_headers_idx.reserve(16);
    m_response_data.reserve(16'384);
}

auto Request::SetUrl(const std::string& url) -> bool
{
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

auto Request::GetUrl() const -> StringView
{
    return m_url;
}

auto Request::SetMethod(
    Method http_method
) -> void
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    switch(http_method)
    {
        case Method::GET:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTPGET, 1L);
            break;
        case Method::HEAD:
            curl_easy_setopt(m_curl_handle, CURLOPT_NOBODY, 1L);
            break;
        case Method::POST:
            curl_easy_setopt(m_curl_handle, CURLOPT_POST, 1L);
            break;
        case Method::PUT:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
            break;
        case Method::DELETE:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case Method::CONNECT:
            curl_easy_setopt(m_curl_handle, CURLOPT_CONNECT_ONLY, 1L);
            break;
        case Method::OPTIONS:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        case Method::PATCH:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
    }
#pragma clang diagnostic pop
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

auto Request::AddHeader(
    StringView name
) -> void
{
    AddHeader(name, StringView());
}

auto Request::AddHeader(
    StringView name,
    StringView value
) -> void
{
    size_t capacity = m_request_headers.capacity();
    size_t header_len = name.length() + value.length() + 3; //": \0"
    size_t total_len = m_request_headers.size() + header_len;
    if(capacity < total_len)
    {
        do
        {
            capacity *= 1.5;
        } while(capacity < total_len);
        m_request_headers.reserve(capacity);
    }

    const char* start = m_request_headers.data() + m_request_headers.size();

    m_request_headers.append(name.data(), name.length());
    m_request_headers.append(": ");
    if(!value.empty())
    {
        m_request_headers.append(value.data(), value.length());
    }
    m_request_headers.append("\0"); // curl expects null byte

    StringView full_header(start, header_len - 1); // subtract off the null byte
    m_request_headers_idx.emplace_back(full_header);
}

auto Request::GetRequestHeaders() const -> const std::vector<Header>&
{
    return m_request_headers_idx;
}

auto Request::SetRequestData(
    std::string data
) -> void
{
    // libcurl expects the data lifetime to be longer
    // than the request so require it to be moved into
    // the lifetime of the request object.
    m_request_data = std::move(data);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
    curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(m_request_data.size()));
    curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDS,    m_request_data.data());
#pragma clang diagnostic pop
}

auto Request::GetRequestData() const -> const std::string&
{
    return m_request_data;
}

auto Request::Perform() -> bool
{
    prepareForPerform();
    auto curl_error_code = curl_easy_perform(m_curl_handle);
    setRequestStatus(curl_error_code);
    return (m_status_code == RequestStatus::SUCCESS);
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

auto Request::GetStatus() const -> RequestStatus
{
    return m_status_code;
}

auto Request::Reset() -> void
{
    m_url = StringView();
    m_request_headers.clear();
    m_request_headers_idx.clear();
    if(m_curl_request_headers)
    {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }
    m_request_data = std::string(); // replace since this buffer is 'moved' into the Request.

    m_status_code = RequestStatus::BUILDING;
    m_response_headers.clear();
    m_response_data.clear();

    curl_easy_reset(m_curl_handle);
    init();
}

auto Request::prepareForPerform() -> void
{
    if(!m_request_headers_idx.empty())
    {
        for(auto header : m_request_headers_idx)
        {
            m_curl_request_headers = curl_slist_append(
                m_curl_request_headers,
                header.GetHeader().data()
            );
        }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, m_curl_request_headers);
#pragma clang diagnostic pop
    }

    m_status_code = RequestStatus::EXECUTING;
}

auto Request::setRequestStatus(
    CURLcode curl_code
) -> void
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
    switch(curl_code)
    {
        case CURLcode::CURLE_OK:
            m_status_code = RequestStatus::SUCCESS;
            break;
        case CURLcode::CURLE_GOT_NOTHING:
            m_status_code = RequestStatus::RESPONSE_EMPTY;
            break;
        case CURLcode::CURLE_OPERATION_TIMEDOUT:
            m_status_code = RequestStatus::TIMEOUT;
            break;
        case CURLcode::CURLE_COULDNT_CONNECT:
            m_status_code = RequestStatus::CONNECT_ERROR;
            break;
        case CURLcode::CURLE_COULDNT_RESOLVE_HOST:
            m_status_code = RequestStatus::CONNECT_DNS_ERROR;
            break;
        case CURLcode::CURLE_SSL_CONNECT_ERROR:
            m_status_code = RequestStatus::CONNECT_SSL_ERROR;
            break;
        default:
            m_status_code = RequestStatus::ERROR;
            break;
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

    size_t capacity = raw_request_ptr->m_response_headers.capacity();
    size_t total_len = raw_request_ptr->m_response_headers.size() + data_view.length();
    if(capacity < total_len)
    {
        do
        {
            capacity *= 1.5;
        } while(capacity < total_len);
        raw_request_ptr->m_response_headers.reserve(capacity);
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
