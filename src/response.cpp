#include "lift/response.hpp"
#include "lift/const.hpp"

#include <functional>

namespace lift
{
response::response()
{
    m_network_error_message[0] = '\0';
    m_headers.reserve(header_default_count);
}

auto response::header(std::string_view name) const -> std::optional<std::reference_wrapper<const lift::header>>
{
    for (const auto& header : m_headers)
    {
        if (header.name() == name)
        {
            return std::optional{std::cref(header)};
        }
    }

    return std::nullopt;
}

std::string_view response::network_error_message() const
{
    return std::string_view(
        m_network_error_message[0] != '\0' ? m_network_error_message : curl_easy_strerror(m_curl_code));
}

auto operator<<(std::ostream& os, const response& r) -> std::ostream&
{
    os << lift::http::to_string(r.m_version) << ' ' << lift::http::to_string(r.m_status_code) << "\r\n";
    for (const auto& header : r.m_headers)
    {
        os << header << "\r\n";
    }
    os << "\r\n";
    if (!r.m_data.empty())
    {
        os << std::string_view{r.m_data.data(), r.m_data.size()};
    }

    return os;
}

} // namespace lift
