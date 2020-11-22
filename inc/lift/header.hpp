#pragma once

#include <string>
#include <string_view>

namespace lift
{
class executor;

class header
{
    friend executor;

public:
    /**
     * Creates an owned header.
     * @param name The name of the header.
     * @param value The value of the header.
     */
    header(std::string_view name, std::string_view value);

    /**
     * Creates an owned header.
     * @param header_full The full "<name>: <value>" header field.
     */
    explicit header(std::string header_full);

    /**
     * @return The entire header, e.g. "Connection: Keep-Alive"
     */
    [[nodiscard]] auto data() const -> const std::string& { return m_header; }

    /**
     * @return The header's name.
     */
    [[nodiscard]] auto name() const -> std::string_view
    {
        std::string_view name = m_header;
        name                  = name.substr(0, m_colon_pos);
        return name;
    }

    /**
     * @return The header's value or empty if it doesn't have a value.
     */
    [[nodiscard]] auto value() const -> std::string_view
    {
        std::string_view value = m_header;
        // Remove the name from the value. We know we made this string with ": " so skip two chars.
        value.remove_prefix(m_colon_pos + 2);
        return value;
    }

    friend auto operator<<(std::ostream& os, const header& h) -> std::ostream&
    {
        os << h.m_header;
        return os;
    }

private:
    /// The full header data.
    std::string m_header{};
    std::size_t m_colon_pos{0};
};

} // namespace lift
