#include "lift/HeaderView.hpp"

#include <string>

namespace lift {

HeaderView::HeaderView(
    std::string_view header_data)
    : m_header(header_data)
    , m_name(header_data)
{
    auto colon_pos = m_name.find(':');
    if (colon_pos != std::string_view::npos) {
        /**
         * Use remove_suffix|prefix for safety bounds checking.
         */

        // Remove the value from the name.
        m_name.remove_suffix(m_name.length() - colon_pos);

        // Remove the name from the value.
        m_value = header_data;
        m_value.remove_prefix(colon_pos + 1);
        // Remove any prefix whitespace.
        while (!m_value.empty() && std::isspace(m_value[0]) != 0) {
            m_value.remove_prefix(1);
        }
    }
}

} // lift
