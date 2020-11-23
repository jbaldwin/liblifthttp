#include "lift/mime_field.hpp"

namespace lift
{
mime_field::mime_field(std::string field_name, std::string field_value)
    : m_field_name(std::move(field_name)),
      m_field_value(std::move(field_value))
{
}

mime_field::mime_field(std::string field_name, std::filesystem::path field_filepath)
    : m_field_name(std::move(field_name)),
      m_field_value(std::move(field_filepath))
{
    // TODO Doesn't seem to work?
    // if (!std::filesystem::exists(field_filepath)) {
    //     throw std::runtime_error("File path doesn't exist.");
    // }
}

} // namespace lift
