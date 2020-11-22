#include "lift/response.hpp"
#include "lift/const.hpp"

namespace lift
{
Response::Response()
{
    m_headers.reserve(header_default_count);
}

auto operator<<(std::ostream& os, const Response& r) -> std::ostream&
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
