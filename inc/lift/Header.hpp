#pragma once

#include <string>
#include <string_view>

namespace lift {

class Executor;

class Header {
    friend Executor;

public:
    /**
     * Creates an owned header.
     * @param name The name of the header.
     * @param value The value of the header.
     */
    explicit Header(
        std::string_view name,
        std::string_view value);

    /**
     * Creates an owned header.
     * @param header_full The full "<name>: <value>" header field.
     */
    explicit Header(
        std::string header_full);

    /**
     * @return The entire header, e.g. "Connection: Keep-Alive"
     */
    [[nodiscard]] auto HeaderFull() const -> const std::string& { return m_header; }

    /**
     * @return The header's name.
     */
    [[nodiscard]] auto Name() const -> std::string_view
    {
        std::string_view name = m_header;
        name = name.substr(0, m_colon_pos);
        return name;
    }

    /**
     * @return The header's value or empty if it doesn't have a value.
     */
    [[nodiscard]] auto Value() const -> std::string_view
    {
        std::string_view value = m_header;
        // Remove the name from the value. We know we made this string with ": " so skip two chars.
        value.remove_prefix(m_colon_pos + 2);
        return value;
    }

private:
    /// The full header data.
    std::string m_header {};
    std::size_t m_colon_pos { 0 };

    // Executor requires a char* to pass into the curlslist.
    [[nodiscard]] auto headerFull() -> std::string& { return m_header; }
};

} // lift
