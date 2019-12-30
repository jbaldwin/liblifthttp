#include "lift/Request.hpp"
#include "lift/RequestHandle.hpp"
#include "lift/RequestPool.hpp"

#include <cstring>

namespace lift {
auto curl_write_header(
    char* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t;

auto curl_write_data(
    void* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t;

auto curl_xfer_info(
    void* clientp,
    curl_off_t download_total_bytes,
    curl_off_t download_now_bytes,
    curl_off_t upload_total_bytes,
    curl_off_t upload_now_bytes) -> int;

static constexpr uint64_t HEADER_DEFAULT_MEMORY_BYTES = 16'384;
static constexpr uint64_t HEADER_DEFAULT_COUNT = 16;

Request::Request(
    RequestPool& request_pool,
    const std::string& url,
    std::chrono::milliseconds timeout,
    std::function<void(RequestHandle)> on_complete_handler)
    : m_on_complete_handler(std::move(on_complete_handler))
    , m_request_pool(request_pool)
{
    init();
    SetUrl(url);
    SetTimeout(timeout);
}

Request::~Request()
{
    Reset();

    // Reset doesn't delete the CURL* handle since it can be re-used between requests.
    // Delete it now.
    if (m_curl_handle != nullptr) {
        curl_easy_cleanup(m_curl_handle);
        m_curl_handle = nullptr;
    }
}

auto Request::init() -> void
{
    curl_easy_setopt(m_curl_handle, CURLOPT_PRIVATE, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, curl_write_header);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    m_request_headers.reserve(HEADER_DEFAULT_MEMORY_BYTES);
    m_request_headers_idx.reserve(HEADER_DEFAULT_COUNT);
    m_headers_committed = false;

    m_response_headers.reserve(HEADER_DEFAULT_MEMORY_BYTES);
    m_response_headers_idx.reserve(HEADER_DEFAULT_COUNT);
    m_response_data.reserve(HEADER_DEFAULT_MEMORY_BYTES);
}

auto Request::SetOnCompleteHandler(
    std::function<void(RequestHandle)> on_complete_handler) -> void
{
    m_on_complete_handler = std::move(on_complete_handler);
}

auto Request::SetUrl(
    const std::string& url) -> bool
{
    if (url.empty()) {
        return false;
    }

    auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_URL, url.c_str());
    if (error_code == CURLE_OK) {
        char* curl_url = nullptr;
        curl_easy_getinfo(m_curl_handle, CURLINFO_EFFECTIVE_URL, &curl_url);
        if (curl_url != nullptr) {
            m_url = std::string_view{ curl_url, std::strlen(curl_url) };
            return true;
        }
    }

    return false;
}

auto Request::GetNumConnects() const -> uint64_t
{
    long count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_NUM_CONNECTS, &count);
    return static_cast<uint64_t>(count);
}

auto Request::GetNumRedirects() const -> uint64_t
{
    long count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_REDIRECT_COUNT, &count);
    return static_cast<uint64_t>(count);
}

auto Request::GetUrl() const -> std::string_view
{
    return m_url;
}

auto Request::SetMethod(
    http::Method http_method) -> void
{
    switch (http_method) {
    case http::Method::GET:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPGET, 1L);
        break;
    case http::Method::HEAD:
        curl_easy_setopt(m_curl_handle, CURLOPT_NOBODY, 1L);
        break;
    case http::Method::POST:
        curl_easy_setopt(m_curl_handle, CURLOPT_POST, 1L);
        break;
    case http::Method::PUT:
        curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
        break;
    case http::Method::DELETE:
        curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
        break;
    case http::Method::CONNECT:
        curl_easy_setopt(m_curl_handle, CURLOPT_CONNECT_ONLY, 1L);
        break;
    case http::Method::OPTIONS:
        curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
        break;
    case http::Method::PATCH:
        curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "PATCH");
        break;
    }
}

auto Request::SetVersion(
    http::Version http_version) -> void
{
    switch (http_version) {
    case http::Version ::USE_BEST:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
        break;
    case http::Version::V1_0:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
        break;
    case http::Version::V1_1:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
        break;
    case http::Version::V2_0:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        break;
    case http::Version::V2_0_TLS:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
        break;
    case http::Version::V2_0_ONLY:
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
        break;
    }
}

auto Request::SetTimeout(
    std::chrono::milliseconds timeout) -> bool
{
    int64_t timeout_ms = timeout.count();

    if (timeout_ms > 0) {
        auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_ms));
        return (error_code == CURLE_OK);
    }
    return false;
}

auto Request::SetFollowRedirects(
    bool follow_redirects,
    int64_t max_redirects) -> bool
{
    long curl_value = (follow_redirects) ? 1L : 0L;
    auto error_code1 = curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, curl_value);
    auto error_code2 = curl_easy_setopt(m_curl_handle, CURLOPT_MAXREDIRS, static_cast<long>(max_redirects));
    return (error_code1 == CURLE_OK && error_code2 == CURLE_OK);
}

auto Request::SetVerifySslPeer(
    bool verify_ssl_peer) -> void
{
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYPEER, (verify_ssl_peer) ? 1L : 0L);
}

auto Request::SetVerifySslHost(
    bool verify_ssl_host) -> void
{
    // Note that 1L is valid, but curl docs say its basically deprecated.
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYHOST.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYHOST, (verify_ssl_host) ? 2L : 0L);
}

auto Request::AddResolveHost(
    ResolveHost resolve_host) -> void
{
    m_resolve_hosts.emplace_back(std::move(resolve_host));
    m_resolve_hosts_committed = false;
}

auto Request::RemoveHeader(
    std::string_view name) -> void
{
    // Curl internally doesn't have a separate list, so just add it to the normal
    // list of headers and let curl remove it.
    AddHeader(name, std::string_view{});
}

auto Request::AddHeader(
    std::string_view name,
    std::string_view value) -> void
{
    m_headers_committed = false; // A new header was added, they need to be committed again.
    size_t capacity = m_request_headers.capacity();
    size_t header_len = name.length() + value.length() + 3; //": \0"
    size_t total_len = m_request_headers.size() + header_len;
    if (capacity < total_len) {
        do {
            capacity *= 2;
        } while (capacity < total_len);
        m_request_headers.reserve(capacity);
    }

    const char* start = m_request_headers.data() + m_request_headers.length();

    m_request_headers.append(name.data(), name.length());
    m_request_headers.append(": ");
    if (!value.empty()) {
        m_request_headers.append(value.data(), value.length());
    }
    m_request_headers += '\0'; // curl expects null byte, do not use string.append, it ignores null terminators!

    std::string_view full_header{ start, header_len - 1 }; // subtract off the null byte
    m_request_headers_idx.emplace_back(full_header);
}

auto Request::GetRequestHeaders() const -> const std::vector<Header>&
{
    return m_request_headers_idx;
}

auto Request::SetRequestData(
    std::string data) -> void
{
    if (m_mime_handle != nullptr) {
        throw std::logic_error("Cannot SetRequestData on Request after using AddMimeField");
    }

    if (data.empty()) {
        return;
    }

    // libcurl expects the data lifetime to be longer
    // than the request so require it to be moved into
    // the lifetime of the request object.
    m_request_data = std::move(data);

    curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(m_request_data.size()));
    curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDS, m_request_data.data());
}

auto Request::GetRequestData() const -> const std::string&
{
    return m_request_data;
}

auto Request::AddMimeField(
    const std::string& field_name,
    const std::string& field_value) -> void
{
    if (!m_request_data.empty()) {
        throw std::logic_error("Cannot AddMimeField on Request after using SetRequestData");
    }

    if (m_mime_handle == nullptr) {
        m_mime_handle = curl_mime_init(m_curl_handle);
    }

    auto* field = curl_mime_addpart(m_mime_handle);

    curl_mime_name(field, field_name.data());
    curl_mime_data(field, field_value.data(), field_value.size());
}

auto Request::AddMimeField(
    const std::string& field_name,
    const std::filesystem::path& field_filepath) -> void
{
    if (!m_request_data.empty()) {
        throw std::logic_error("Cannot AddMimeField on Request after using SetRequestData");
    }

    if (!std::filesystem::exists(field_filepath)) {
        throw std::runtime_error("Filepath for AddMimeField doesn't exist");
    }

    if (m_mime_handle == nullptr) {
        m_mime_handle = curl_mime_init(m_curl_handle);
    }

    auto* field = curl_mime_addpart(m_mime_handle);

    curl_mime_filename(field, field_name.data());
    curl_mime_filedata(field, field_filepath.c_str());
}

auto Request::SetTransferProgressHandler(
    std::optional<TransferProgressHandler> transfer_progress_handler) -> void
{
    if (transfer_progress_handler.has_value() && transfer_progress_handler.value()) {
        m_on_transfer_progress_handler = std::move(transfer_progress_handler.value());
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFOFUNCTION, curl_xfer_info);
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 0L);
    } else {
        m_on_transfer_progress_handler = nullptr;
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 1L);
    }
}

auto Request::Perform() -> bool
{
    prepareForPerform();
    auto curl_error_code = curl_easy_perform(m_curl_handle);
    setCompletionStatus(curl_error_code);
    return (m_status_code == RequestStatus::SUCCESS);
}

auto Request::GetResponseStatusCode() const -> http::StatusCode
{
    long http_response_code = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &http_response_code);
    return http::to_enum(static_cast<uint32_t>(http_response_code));
}

auto Request::GetResponseHeaders() const -> const std::vector<Header>&
{
    return m_response_headers_idx;
}

auto Request::GetResponseData() const -> const std::string&
{
    return m_response_data;
}

auto Request::GetTotalTime() const -> std::chrono::milliseconds
{
    constexpr uint64_t SEC_2_MS = 1000;

    double total_time = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_TOTAL_TIME, &total_time);
    return std::chrono::milliseconds{ static_cast<int64_t>(total_time * SEC_2_MS) };
}

auto Request::GetCompletionStatus() const -> RequestStatus
{
    return m_status_code;
}

auto Request::Reset() -> void
{
    m_url = std::string_view{};
    m_request_headers.clear();
    m_request_headers_idx.clear();
    if (m_curl_request_headers != nullptr) {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }
    if (m_curl_resolve_hosts != nullptr) {
        curl_slist_free_all(m_curl_resolve_hosts);
        m_curl_resolve_hosts = nullptr;
    }

    // replace rather than clear() since this buffer is 'moved' into the RequestHandle and will free up memory.
    m_request_data = std::string{};

    if (m_mime_handle != nullptr) {
        curl_mime_free(m_mime_handle);
        m_mime_handle = nullptr;
    }

    clearResponseBuffers();

    m_on_transfer_progress_handler = nullptr;

    curl_easy_reset(m_curl_handle);
    init();
    m_status_code = RequestStatus::BUILDING;
}

auto Request::prepareForPerform() -> void
{
    clearResponseBuffers();
    if (!m_headers_committed && !m_request_headers_idx.empty()) {
        // Its possible the headers have been previous committed -- this will re-commit them all
        // in the event additional headers have been added between requests.
        if (m_curl_request_headers != nullptr) {
            curl_slist_free_all(m_curl_request_headers);
            m_curl_request_headers = nullptr;
        }

        for (const auto& header : m_request_headers_idx) {
            m_curl_request_headers = curl_slist_append(
                m_curl_request_headers,
                header.GetHeader().data());
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, m_curl_request_headers);
        m_headers_committed = true;
    }

    if (m_mime_handle != nullptr) {
        curl_easy_setopt(m_curl_handle, CURLOPT_MIMEPOST, m_mime_handle);
    }

    // Commit the resolve hosts from the request + any pool level resolve hosts.
    if (!m_resolve_hosts_committed && (!m_resolve_hosts.empty() || !m_request_pool.m_resolve_hosts.empty())) {
        if (m_curl_resolve_hosts != nullptr) {
            curl_slist_free_all(m_curl_resolve_hosts);
            m_curl_resolve_hosts = nullptr;
        }

        for (const auto& resolve_host : m_resolve_hosts) {
            m_curl_resolve_hosts = curl_slist_append(
                m_curl_resolve_hosts, resolve_host.getCurlFormattedResolveHost().data());
        }

        for (const auto& resolve_host : m_request_pool.m_resolve_hosts) {
            m_curl_resolve_hosts = curl_slist_append(
                m_curl_resolve_hosts, resolve_host.getCurlFormattedResolveHost().data());
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_RESOLVE, m_curl_resolve_hosts);
        m_resolve_hosts_committed = true;
    }

    m_status_code = RequestStatus::EXECUTING;
}

auto Request::clearResponseBuffers() -> void
{
    m_response_headers.clear();
    m_response_headers_idx.clear();
    m_response_data.clear();
}

auto Request::setCompletionStatus(
    CURLcode curl_code) -> void
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (curl_code) {
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
    case CURLcode::CURLE_WRITE_ERROR:
        m_status_code = RequestStatus::DOWNLOAD_ERROR;
        break;
    case CURLcode::CURLE_SEND_ERROR:
        m_status_code = RequestStatus::ERROR_FAILED_TO_START;
        break;
    default:
        m_status_code = RequestStatus::ERROR;
        break;
    }
#pragma GCC diagnostic pop
}

auto curl_write_header(
    char* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t
{
    auto* raw_request_ptr = static_cast<Request*>(user_ptr);
    const size_t data_length = size * nitems;

    std::string_view data_view{ buffer, data_length };

    if (data_view.empty()) {
        return data_length;
    }

    // Ignore empty header lines from curl.
    if (data_length == 2 && data_view == "\r\n") {
        return data_length;
    }
    // Ignore the HTTP/ 'header' line from curl.
    constexpr size_t HTTPSLASH_LEN = 5;
    if (data_length >= 4 && data_view.substr(0, HTTPSLASH_LEN) == "HTTP/") {
        return data_length;
    }

    // Drop the trailing \r\n from the header.
    if (data_length >= 2) {
        size_t rm_size = (data_view[data_length - 1] == '\n' && data_view[data_length - 2] == '\r') ? 2 : 0;
        data_view.remove_suffix(rm_size);
    }

    const auto cleaned_up_length = data_view.length();

    size_t capacity = raw_request_ptr->m_response_headers.capacity();
    size_t total_len = raw_request_ptr->m_response_headers.size() + cleaned_up_length;
    if (capacity < total_len) {
        do {
            capacity *= 2;
        } while (capacity < total_len);
        raw_request_ptr->m_response_headers.reserve(capacity);
    }

    // Append the entire header into the full header buffer.
    raw_request_ptr->m_response_headers.append(data_view.data(), cleaned_up_length);

    // Calculate and append the Header view object.
    const char* start = raw_request_ptr->m_response_headers.c_str();
    auto total_length = raw_request_ptr->m_response_headers.length();
    std::string_view request_data_view{ (start + total_length) - cleaned_up_length, cleaned_up_length };
    raw_request_ptr->m_response_headers_idx.emplace_back(request_data_view);

    return data_length; // return original size for curl to continue processing
}

auto curl_write_data(
    void* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t
{
    auto* raw_request_ptr = static_cast<Request*>(user_ptr);
    size_t data_length = size * nitems;

    raw_request_ptr->m_response_data.append(static_cast<const char*>(buffer), data_length);

    return data_length;
}

auto curl_xfer_info(
    void* clientp,
    curl_off_t download_total_bytes,
    curl_off_t download_now_bytes,
    curl_off_t upload_total_bytes,
    curl_off_t upload_now_bytes) -> int
{
    const auto* raw_request_ptr = static_cast<const Request*>(clientp);

    if (raw_request_ptr != nullptr && raw_request_ptr->m_on_transfer_progress_handler) {
        if (raw_request_ptr->m_on_transfer_progress_handler(
                *raw_request_ptr,
                download_total_bytes,
                download_now_bytes,
                upload_total_bytes,
                upload_now_bytes)) {
            return 0;
        } else {
            return 1;
        }
    } else {
        return 0; // continue the request.
    }
}

} // lift
