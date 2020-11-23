#include "lift/query_builder.hpp"
#include "lift/escape.hpp"

namespace lift
{
auto query_builder::scheme(std::string_view scheme) -> query_builder&
{
    m_scheme = scheme;
    return *this;
}

auto query_builder::hostname(std::string_view hostname) -> query_builder&
{
    m_hostname = hostname;
    return *this;
}

auto query_builder::port(uint16_t port) -> query_builder&
{
    m_port = port;
    return *this;
}

auto query_builder::append_path_part(std::string_view path_part) -> query_builder&
{
    m_path_parts.emplace_back(path_part);
    return *this;
}

auto query_builder::append_query_parameter(std::string_view name, std::string_view value) -> query_builder&
{
    m_query_parameters.emplace_back(name, value);
    return *this;
}

auto query_builder::fragment(std::string_view fragment) -> query_builder&
{
    m_fragment = fragment;
    return *this;
}

auto query_builder::build() -> std::string
{
    if (!m_scheme.empty())
    {
        m_query << m_scheme;
    }
    m_query << "://";
    if (!m_hostname.empty())
    {
        m_query << m_hostname;
    }
    if (m_port != 0)
    {
        m_query << ":" << m_port;
    }
    if (!m_path_parts.empty())
    {
        for (auto path_part : m_path_parts)
        {
            m_query << "/" << path_part;
        }
    }
    if (!m_query_parameters.empty())
    {
        bool first = true;

        for (const auto& [name, value] : m_query_parameters)
        {
            if (first)
            {
                m_query << "?";
                first = false;
            }
            else
            {
                m_query << "&";
            }
            auto escaped_value = lift::escape(value);
            m_query << name << "=" << escaped_value;
        }
    }
    if (!m_fragment.empty())
    {
        m_query << "#" << m_fragment;
    }

    auto copy = m_query.str();
    reset();
    return copy;
}

auto query_builder::reset() -> void
{
    m_query.clear();
    m_query.str("");
    m_scheme   = std::string_view{};
    m_hostname = std::string_view{};
    m_port     = 0;
    m_path_parts.clear();
    m_query_parameters.clear();
    m_fragment = std::string_view{};
}

} // namespace lift
