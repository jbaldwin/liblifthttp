#pragma once

#include <filesystem>
#include <string>
#include <variant>

namespace lift
{
class mime_field
{
public:
    mime_field(std::string field_name, std::string field_value);

    /**
     * @throw std::runtime_error If field_filepath does not exist on disk.
     */
    mime_field(std::string field_name, std::filesystem::path field_filepath);

    auto name() const -> const std::string& { return m_field_name; }
    auto value() const -> const std::variant<std::string, std::filesystem::path>& { return m_field_value; }

private:
    std::string                                      m_field_name{};
    std::variant<std::string, std::filesystem::path> m_field_value{};
};

} // namespace lift
