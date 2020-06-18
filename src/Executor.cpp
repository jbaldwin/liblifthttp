#include "lift/Executor.hpp"
#include "lift/EventLoop.hpp"
#include "lift/Init.hpp"

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

Executor::Executor(
    Request* request,
    Share* share)
    : m_request_sync(request)
    , m_request(m_request_sync)
{
    if (share != nullptr) {
        m_curl_share_handle = share->m_curl_share_ptr;
    }
}

Executor::Executor(
    EventLoop* event_loop)
    : m_event_loop(event_loop)
{
}

Executor::~Executor()
{
    reset();
    curl_easy_cleanup(m_curl_handle);
}

auto Executor::startAsync(
    RequestPtr request_ptr,
    Share* share) -> void
{
    m_request_async = std::move(request_ptr);
    m_request = m_request_async.get();
    if (share != nullptr) {
        m_curl_share_handle = share->m_curl_share_ptr;
    }
}

auto Executor::perform() -> Response
{
    global_init();

    prepare();

    auto curl_error_code = curl_easy_perform(m_curl_handle);
    m_response.m_lift_status = convert(curl_error_code);
    copyCurlToResponse();

    global_cleanup();

    return std::move(m_response);
}

auto Executor::prepare() -> void
{
    curl_easy_setopt(m_curl_handle, CURLOPT_PRIVATE, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, curl_write_header);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1L);

    curl_easy_setopt(m_curl_handle, CURLOPT_URL, m_request->Url().c_str());

    switch (m_request->Method()) {
    case http::Method::UNKNOWN: // default to GET on unknown/bad value.
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
        curl_easy_setopt(m_curl_handle, CURLOPT_PUT, 1L);
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

    switch (m_request->Version()) {
    case http::Version::UNKNOWN: // default to USE_BEST on unknown/bad value.
    case http::Version::USE_BEST:
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

    // Synchronous Requests get their timeout value set directly on the curl easy handle.
    // Asynchronous requests will handle timeouts on the event loop due to Connection Time.
    if (m_request_sync != nullptr) {
        if (m_request->Timeout().has_value()) {
            curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(m_request->Timeout().value().count()));
        }
    }

    // Connection timeout is handled when injecting into the CURLM* event loop for asynchronous requests.

    if (m_request->FollowRedirects()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl_handle, CURLOPT_MAXREDIRS, static_cast<long>(m_request->MaxRedirects()));
    } else {
        curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 0L);
    }

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYPEER, (m_request->VerifySslPeer()) ? 1L : 0L);
    // Note that 1L is valid, but curl docs say its basically deprecated.
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYHOST.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYHOST, (m_request->VerifySslHost()) ? 2L : 0L);
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYSTATUS.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYSTATUS, (m_request->VerifySslStatus()) ? 1L : 0L);

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERT.html
    if (const auto& cert = m_request->SslCert(); cert.has_value()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLCERT, cert.value().c_str());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERTTYPE.html
    if (const auto& cert_type = m_request->SslCertType(); cert_type.has_value()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLCERTTYPE, to_string(cert_type.value()).data());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLKEY.html
    if (const auto& key = m_request->SslKey(); key.has_value()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLKEY, key.value().c_str());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_KEYPASSWD.html
    if (const auto& password = m_request->KeyPassword(); password.has_value()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_KEYPASSWD, password.value().data());
    }

    const auto& encodings = m_request->AcceptEncodings();
    if (encodings.has_value()) {
        if (!encodings.value().empty()) {
            std::size_t length { 0 };
            for (const auto& e : encodings.value()) {
                length += e.length() + 2; // for ", "
            }

            std::string joined {};
            joined.reserve(length);

            bool first { true };
            for (auto& e : encodings.value()) {
                if (first) {
                    first = false;
                } else {
                    joined.append(", ");
                }
                joined.append(e);
            }

            // strings are copied into libcurl except for POSTFIELDS.
            curl_easy_setopt(m_curl_handle, CURLOPT_ACCEPT_ENCODING, joined.c_str());
        } else {
            // From the CURL docs (https://curl.haxx.se/libcurl/c/CURLOPT_ACCEPT_ENCODING.html):
            // 'To aid applications not having to bother about what specific algorithms this particular
            // libcurl build supports, libcurl allows a zero-length string to be set ("") to ask for an
            // Accept-Encoding: header to be used that contains all built-in supported encodings.'
            curl_easy_setopt(m_curl_handle, CURLOPT_ACCEPT_ENCODING, "");
        }
    }

    // Headers
    if (m_curl_request_headers != nullptr) {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }

    for (auto& header : m_request->m_request_headers) {
        m_curl_request_headers = curl_slist_append(
            m_curl_request_headers, header.headerFull().data());
    }

    if (m_curl_request_headers != nullptr) {
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, m_curl_request_headers);
    } else {
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, nullptr);
    }

    // DNS resolve hosts
    if (!m_request->m_resolve_hosts.empty()
        || (m_event_loop != nullptr && !m_event_loop->m_resolve_hosts.empty())) {

        if (m_curl_resolve_hosts != nullptr) {
            curl_slist_free_all(m_curl_resolve_hosts);
            m_curl_resolve_hosts = nullptr;
        }

        for (const auto& resolve_host : m_request->m_resolve_hosts) {
            m_curl_resolve_hosts = curl_slist_append(
                m_curl_resolve_hosts, resolve_host.getCurlFormattedResolveHost().data());
        }

        if (m_event_loop != nullptr) {
            for (const auto& resolve_host : m_event_loop->m_resolve_hosts) {
                m_curl_resolve_hosts = curl_slist_append(
                    m_curl_resolve_hosts, resolve_host.getCurlFormattedResolveHost().data());
            }
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_RESOLVE, m_curl_resolve_hosts);
    }

    // POST or MIME data
    if (m_request->m_request_data_set) {
        curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(m_request->Data().size()));
        curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDS, m_request->Data().data());
    } else if (m_request->m_mime_fields_set) {
        m_mime_handle = curl_mime_init(m_curl_handle);

        for (const auto& mime_field : m_request->MimeFields()) {
            auto* field = curl_mime_addpart(m_mime_handle);

            if (std::holds_alternative<std::string>(mime_field.Value())) {
                curl_mime_name(field, mime_field.Name().data());
                const auto& value = std::get<std::string>(mime_field.Value());
                curl_mime_data(field, value.data(), value.length());
            } else {
                curl_mime_filename(field, mime_field.Name().data());
                curl_mime_filedata(field, std::get<std::filesystem::path>(mime_field.Value()).c_str());
            }
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_MIMEPOST, m_mime_handle);
    }

    if (m_request->m_on_transfer_progress_handler != nullptr) {
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFOFUNCTION, curl_xfer_info);
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 0L);
    } else {
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 1L);
    }

    if (const auto& timeout = m_request->HappyEyeballsTimeout(); timeout.has_value()) {
        curl_easy_setopt(m_curl_handle, CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS, static_cast<long>(timeout.value().count()));
    }

    // Note that this will lock the mutexes in the share callbacks.
    if (m_curl_share_handle != nullptr) {
        curl_easy_setopt(m_curl_handle, CURLOPT_SHARE, m_curl_share_handle);
    }
}

auto Executor::copyCurlToResponse() -> void
{
    long http_response_code = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &http_response_code);
    m_response.m_status_code = http::to_enum(static_cast<int32_t>(http_response_code));

    double total_time = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_TOTAL_TIME, &total_time);
    // std::duration defaults to seconds, so don't need to duration_cast total time to seconds.
    m_response.m_total_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double> { total_time });

    long connect_count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_NUM_CONNECTS, &connect_count);
    m_response.m_num_connects = static_cast<uint64_t>(connect_count);

    long redirect_count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_REDIRECT_COUNT, &redirect_count);
    m_response.m_num_redirects = static_cast<uint64_t>(redirect_count);
}

auto Executor::setTimesupResponse(
    std::chrono::milliseconds total_time) -> void
{
    m_response.m_status_code = lift::http::StatusCode::HTTP_504_GATEWAY_TIMEOUT;
    m_response.m_total_time = total_time;
    m_response.m_num_connects = 0;
    m_response.m_num_redirects = 0;
}

auto Executor::reset() -> void
{
    if (m_mime_handle != nullptr) {
        curl_mime_free(m_mime_handle);
        m_mime_handle = nullptr;
    }

    if (m_curl_request_headers != nullptr) {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }

    if (m_curl_resolve_hosts != nullptr) {
        curl_slist_free_all(m_curl_resolve_hosts);
        m_curl_resolve_hosts = nullptr;
    }

    // Regardless of sync/async all three pointers get reset to nullptr.
    m_request_sync = nullptr;
    m_request_async = nullptr;
    m_request = nullptr;

    m_timeout_iterator.reset();
    m_on_complete_callback_called = false;
    m_response = Response {};

    curl_easy_setopt(m_curl_handle, CURLOPT_SHARE, nullptr);
    m_curl_share_handle = nullptr;

    curl_easy_reset(m_curl_handle);
}

auto Executor::convert(
    CURLcode curl_code) -> LiftStatus
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (curl_code) {
    case CURLcode::CURLE_OK:
        return LiftStatus::SUCCESS;
    case CURLcode::CURLE_GOT_NOTHING:
        return LiftStatus::RESPONSE_EMPTY;
    case CURLcode::CURLE_OPERATION_TIMEDOUT:
        return LiftStatus::TIMEOUT;
    case CURLcode::CURLE_COULDNT_CONNECT:
        return LiftStatus::CONNECT_ERROR;
    case CURLcode::CURLE_COULDNT_RESOLVE_HOST:
        return LiftStatus::CONNECT_DNS_ERROR;
    case CURLcode::CURLE_SSL_CONNECT_ERROR:
        return LiftStatus::CONNECT_SSL_ERROR;
    case CURLcode::CURLE_WRITE_ERROR:
        return LiftStatus::DOWNLOAD_ERROR;
    case CURLcode::CURLE_SEND_ERROR:
        return LiftStatus::ERROR_FAILED_TO_START;
    default:
        return LiftStatus::ERROR;
    }
#pragma GCC diagnostic pop
}

auto curl_write_header(
    char* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t
{
    auto* executor_ptr = static_cast<Executor*>(user_ptr);
    auto& response = executor_ptr->m_response;
    const size_t data_length = size * nitems;

    std::string_view data_view { buffer, data_length };

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

    response.m_headers.emplace_back(std::string { data_view.data(), data_view.length() });

    return data_length; // return original size for curl to continue processing
}

auto curl_write_data(
    void* buffer,
    size_t size,
    size_t nitems,
    void* user_ptr) -> size_t
{
    auto* executor_ptr = static_cast<Executor*>(user_ptr);
    auto& response = executor_ptr->m_response;
    size_t data_length = size * nitems;

    response.m_data.append(static_cast<const char*>(buffer), data_length);

    return data_length;
}

auto curl_xfer_info(
    void* clientp,
    curl_off_t download_total_bytes,
    curl_off_t download_now_bytes,
    curl_off_t upload_total_bytes,
    curl_off_t upload_now_bytes) -> int
{
    const auto* executor_ptr = static_cast<const Executor*>(clientp);

    if (executor_ptr != nullptr
        && executor_ptr->m_request->m_on_transfer_progress_handler != nullptr) {
        if (executor_ptr->m_request->m_on_transfer_progress_handler(
                *executor_ptr->m_request,
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

} // namespace lift
