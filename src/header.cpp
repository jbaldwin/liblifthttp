#include "lift/header.hpp"

#include <string>

namespace lift
{
header::header(std::string_view name, std::string_view value)
{
    m_header.reserve(name.length() + value.length() + 2);
    m_header.append(name.data(), name.length());
    m_header.append(": ");
    m_header.append(value.data(), value.length());

    m_colon_pos = name.length();
}

header::header(std::string header_full) : m_header(std::move(header_full))
{
    m_colon_pos = m_header.find(":");
    // class assumes the two bytes ": " always exist, enforce that.
    if (m_colon_pos == std::string::npos)
    {
        m_colon_pos = m_header.length();
        m_header.append(": ");
    }
    else if (m_colon_pos == m_header.length() - 1)
    {
        m_header.append(" ");
    }
    else if (m_header[m_colon_pos + 1] != ' ')
    {
        m_header.insert(m_colon_pos + 1, 1, ' ');
    }
}

} // namespace lift
