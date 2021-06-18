#include "lift/request.hpp"
#include "lift/const.hpp"
#include "lift/executor.hpp"

namespace lift
{
using namespace std::string_literals;
static const std::string ssl_cert_type_unknown = "unknown"s;
static const std::string ssl_cert_type_pem     = "PEM"s;
static const std::string ssl_cert_type_der     = "DER"s;

auto to_string(ssl_certificate_type type) -> const std::string&
{
    switch (type)
    {
        case ssl_certificate_type::pem:
            return ssl_cert_type_pem;
        case ssl_certificate_type::der:
            return ssl_cert_type_der;
        default:
            return ssl_cert_type_unknown;
    }
}

static const std::string debug_info_type_unknown      = "unknown"s;
static const std::string debug_info_type_text         = "text"s;
static const std::string debug_info_type_header_in    = "header_in"s;
static const std::string debug_info_type_header_out   = "header_out"s;
static const std::string debug_info_type_data_in      = "data_in"s;
static const std::string debug_info_type_data_out     = "data_out"s;
static const std::string debug_info_type_ssl_data_out = "ssl_data_out"s;
static const std::string debug_info_type_ssl_data_in  = "ssl_data_in"s;

auto to_string(debug_info_type type) -> const std::string&
{
    switch (type)
    {
        case debug_info_type::text:
            return debug_info_type_text;
        case debug_info_type::header_in:
            return debug_info_type_header_in;
        case debug_info_type::header_out:
            return debug_info_type_header_out;
        case debug_info_type::data_in:
            return debug_info_type_data_in;
        case debug_info_type::data_out:
            return debug_info_type_data_out;
        case debug_info_type::ssl_data_out:
            return debug_info_type_ssl_data_out;
        case debug_info_type::ssl_data_in:
            return debug_info_type_ssl_data_in;
        default:
            return debug_info_type_unknown;
    }
}

request::request(std::string url, std::optional<std::chrono::milliseconds> timeout)
    : m_timeout(std::move(timeout)),
      m_url(std::move(url))
{
}

auto request::perform(share_ptr share_ptr) -> response
{
    executor exe{this, share_ptr.get()};
    return exe.perform();
}

auto request::transfer_progress_handler(std::optional<transfer_progress_handler_type> transfer_progress_handler) -> void
{
    if (transfer_progress_handler.has_value() && transfer_progress_handler.value())
    {
        m_on_transfer_progress_handler = std::move(transfer_progress_handler.value());
    }
    else
    {
        m_on_transfer_progress_handler = nullptr;
    }
}

auto request::follow_redirects(bool follow_redirects, std::optional<uint64_t> max_redirects) -> void
{
    if (follow_redirects)
    {
        m_follow_redirects = true;
        if (max_redirects.has_value())
        {
            m_max_redirects = static_cast<int64_t>(max_redirects.value());
        }
        else
        {
            // curl uses -1 as infinite.
            m_max_redirects = -1;
        }
    }
    else
    {
        m_follow_redirects = false;
        // curl uses 0 as no redirects.
        m_max_redirects = 0;
    }
}

auto request::header(std::string_view name, std::string_view value) -> void
{
    m_request_headers.emplace_back(name, value);
}

auto request::data(std::string data) -> void
{
    if (m_mime_fields_set)
    {
        throw std::logic_error("Cannot set POST request data on request after using adding Mime Fields.");
    }

    m_request_data_set = true;
    m_request_data     = std::move(data);
    // Attempt to switch to a smarter verb if it isn't already set.
    if (m_method != http::method::post || m_method != http::method::put)
    {
        m_method = http::method::post;
    }
}

auto request::mime_field(lift::mime_field mf) -> void
{
    if (m_request_data_set)
    {
        throw std::logic_error("Cannot add Mime Fields on request after using POST request data.");
    }

    m_mime_fields_set = true;
    m_mime_fields.emplace_back(std::move(mf));
}

} // namespace lift
