#include "lift/Request.hpp"
#include "lift/Const.hpp"
#include "lift/Executor.hpp"

namespace lift {

using namespace std::string_literals;
static const std::string SSL_CERT_TYPE_UNKNOWN = "UNKNOWN"s;
static const std::string SSL_CERT_TYPE_PEM = "PEM"s;
static const std::string SSL_CERT_TYPE_DER = "DER"s;

auto to_string(
    SslCertificateType type) -> const std::string&
{
    switch (type) {
    case SslCertificateType::PEM:
        return SSL_CERT_TYPE_PEM;
    case SslCertificateType::DER:
        return SSL_CERT_TYPE_DER;
    default:
        return SSL_CERT_TYPE_UNKNOWN;
    }
}

Request::Request(
    std::string url,
    std::optional<std::chrono::milliseconds> timeout,
    OnCompleteHandlerType on_complete_handler)
    : m_url(std::move(url))
    , m_timeout(std::move(timeout))
    , m_on_complete_handler(std::move(on_complete_handler))
{
}

auto Request::Perform(
    SharePtr share_ptr) -> Response
{
    Executor executor { this, share_ptr.get() };
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

auto Request::FollowRedirects(
    bool follow_redirects,
    std::optional<uint64_t> max_redirects) -> void
{
    if (follow_redirects) {
        m_follow_redirects = true;
        if (max_redirects.has_value()) {
            m_max_redirects = max_redirects.value();
        } else {
            // curl uses -1 as infinite.
            m_max_redirects = -1;
        }
    } else {
        m_follow_redirects = false;
        // curl uses 0 as no redirects.
        m_max_redirects = 0;
    }
}

auto Request::Header(
    std::string_view name,
    std::string_view value) -> void
{
    m_request_headers.emplace_back(name, value);
}

auto Request::Data(
    std::string data) -> void
{
    if (m_mime_fields_set) {
        throw std::logic_error("Cannot set POST request data on Request after using adding Mime Fields.");
    }

    m_request_data_set = true;
    m_request_data = std::move(data);
    m_method = http::Method::POST;
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
