#include "lift/Response.hpp"
#include "lift/Const.hpp"

namespace lift {

Response::Response()
{
    m_headers.reserve(HEADER_DEFAULT_COUNT);
    m_data.reserve(HEADER_DEFAULT_MEMORY_BYTES);
}

auto operator<<(std::ostream& os, const Response& r) -> std::ostream&
{
    os << lift::http::to_string(r.m_version) << ' ' << lift::http::to_string(r.m_status_code) << "\r\n";
    for(const auto& header : r.m_headers)
    {
        os << header.HeaderFull() << "\r\n";
    }
    os << "\r\n";
    os << r.m_data;

    return os;
}

} // namespace lift
