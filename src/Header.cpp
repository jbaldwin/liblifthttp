#include "lift/Header.h"

namespace lift
{

Header::Header(
    StringView header_data
)
    : m_header(header_data),
      m_name(header_data),
      m_value()
{
    auto colon_pos = m_name.find(':');
    if(colon_pos != StringView::npos)
    {
        /**
         * Use remove_suffix|prefix for safety bounds checking.
         */

        // Remove the value from the name.
        m_name.remove_suffix(m_name.length() - colon_pos);

        // Remove the name from the value.
        m_value = header_data;
        m_value.remove_prefix(colon_pos + 1);
        // Remove any prefix whitespace.
        while(!m_value.empty() && std::isspace(m_value[0]))
        {
            m_value.remove_prefix(1);
        }
    }

}

auto Header::GetHeader() const -> StringView
{
    return m_header;
}

auto Header::GetName() const -> StringView
{
    return m_name;
}

auto Header::HasValue() const -> bool
{
    return !m_value.empty();
}

auto Header::GetValue() const -> StringView
{
    return m_value;
}

} // lift

