#pragma once

#include <string_view>

namespace lift {

class HeaderView {
public:
    /**
     * Creates a view into the header data.
     * @param header_data The full header data to split into name/value pair.
     */
    explicit HeaderView(
        std::string_view header_data);

    /**
     * @return The entire header, e.g. "Connection: Keep-Alive"
     */
    [[nodiscard]] auto Header() const -> std::string_view { return m_header; }

    /**
     * @return A view into the header's name.
     */
    [[nodiscard]] auto Name() const -> std::string_view { return m_name; }

    /**
     * @return True if this header has a value.
     */
    [[nodiscard]] auto HasValue() const -> bool { return !m_value.empty(); }

    /**
     * @return A view into the header's value or empty if it doesn't have a value.
     */
    [[nodiscard]] auto Value() const -> std::string_view { return m_value; }

private:
    /// The full header data.
    std::string_view m_header;
    /// The header's name.
    std::string_view m_name;
    /// The header's value if it has one.
    std::string_view m_value {};
};

} // lift
