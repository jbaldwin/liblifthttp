#include "lift/Request.hpp"
#include "lift/Const.hpp"
#include "lift/Executor.hpp"

namespace lift {

Request::Request(
    std::string url,
    std::optional<std::chrono::milliseconds> timeout,
    OnCompleteHandlerType on_complete_handler)
    : m_url(std::move(url))
    , m_timeout(std::move(timeout))
    , m_on_complete_handler(std::move(on_complete_handler))
{
    m_request_headers.reserve(HEADER_DEFAULT_MEMORY_BYTES);
    m_request_headers_idx.reserve(HEADER_DEFAULT_COUNT);
}

auto Request::Perform() -> Response
{
    Executor executor{ this };
    return executor.perform();
}

auto Request::OnCompleteHandler(
    OnCompleteHandlerType on_complete_handler) -> void
{
    m_on_complete_handler = std::move(on_complete_handler);
}

auto Request::TransferProgressHandler(
    std::optional<TransferProgressHandlerType> transfer_progress_handler) -> void
{
    if (transfer_progress_handler.has_value() && transfer_progress_handler.value()) {
        m_on_transfer_progress_handler = std::move(transfer_progress_handler.value());
    } else {
        m_on_transfer_progress_handler = nullptr;
    }
}

auto Request::Timeout() const -> const std::optional<std::chrono::milliseconds>&
{
    return m_timeout;
}

auto Request::Timeout(
    std::optional<std::chrono::milliseconds> timeout) -> void
{
    m_timeout = std::move(timeout);
}

auto Request::Url() const -> const std::string&
{
    return m_url;
}

auto Request::Url(
    std::string url) -> void
{
    m_url = std::move(url);
}

auto Request::Method() const -> http::Method
{
    return m_method;
}

auto Request::Method(
    http::Method method) -> void
{
    m_method = method;
}

auto Request::Version() const -> http::Version
{
    return m_version;
}

auto Request::Version(
    http::Version version) -> void
{
    m_version = version;
}

auto Request::FollowRedirects() const -> bool
{
    return m_follow_redirects;
}

auto Request::MaxRedirects() const -> int64_t
{
    return m_max_redirects;
}

auto Request::FollowRedirects(
    bool follow_redirects,
    int64_t max_redirects) -> void
{
    if (follow_redirects) {
        m_follow_redirects = true;
        if (max_redirects >= -1) {
            m_max_redirects = max_redirects;
        } else {
            // treat any negative number as -1 (infinite).
            m_max_redirects = -1;
        }
    } else {
        m_follow_redirects = false;
    }
}

auto Request::VerifySslPeer() const -> bool
{
    return m_verify_ssl_peer;
}

auto Request::VerifySslPeer(
    bool verify_ssl_peer) -> void
{
    m_verify_ssl_peer = verify_ssl_peer;
}

auto Request::VerifySslHost() const -> bool
{
    return m_verify_ssl_host;
}

auto Request::VerifySslHost(
    bool verify_ssl_host) -> void
{
    m_verify_ssl_host = verify_ssl_host;
}

auto Request::AcceptEncodings() const -> const std::optional<std::vector<std::string>>&
{
    return m_accept_encodings;
}
auto Request::AcceptEncoding(
    std::optional<std::vector<std::string>> encodings) -> void
{
    m_accept_encodings = std::move(encodings);
}

auto Request::ResolveHosts() const -> const std::vector<lift::ResolveHost>&
{
    return m_resolve_hosts;
}

auto Request::ResolveHost(
    lift::ResolveHost resolve_host) -> void
{
    m_resolve_hosts.emplace_back(std::move(resolve_host));
}

auto Request::RemoveHeader(
    std::string_view name) -> void
{
    Header(name, std::string_view{});
}

auto Request::Header(
    std::string_view name,
    std::string_view value) -> void
{
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

auto Request::Headers() const -> const std::vector<HeaderView>&
{
    return m_request_headers_idx;
}

auto Request::RequestData() const -> const std::string&
{
    return m_request_data;
}

auto Request::RequestData(
    std::string data) -> void
{
    if (m_mime_fields_set) {
        throw std::logic_error("Cannot set POST request data on Request after using adding Mime Fields.");
    }

    m_request_data_set = true;
    m_request_data = std::move(data);
    m_method = http::Method::POST;
}

auto Request::MimeFields() const -> const std::vector<lift::MimeField>&
{
    return m_mime_fields;
}

auto Request::MimeField(
    lift::MimeField mime_field) -> void
{
    if (m_request_data_set) {
        throw std::logic_error("Cannot add Mime Fields on Request after using POST request data.");
    }

    m_mime_fields_set = true;
    m_mime_fields.emplace_back(std::move(mime_field));
}

} // namespace lift
