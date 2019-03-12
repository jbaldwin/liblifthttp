#include "lift/QueryBuilder.h"
#include "lift/Escape.h"

namespace lift
{

QueryBuilder::QueryBuilder()
    :   m_query(),
        m_scheme(),
        m_hostname(),
        m_port(0),
        m_path_parts(),
        m_query_parameters(),
        m_fragment()
{

}

auto QueryBuilder::SetScheme(
    std::string_view scheme
) -> QueryBuilder&
{
    m_scheme = scheme;
    return *this;
}

auto QueryBuilder::SetHostname(
    std::string_view hostname
) -> QueryBuilder&
{
    m_hostname = hostname;
    return *this;
}

auto QueryBuilder::SetPort(
    uint16_t port
) -> QueryBuilder&
{
    m_port = port;
    return *this;
}

auto QueryBuilder::AppendPathPart(
    std::string_view path_part
) -> QueryBuilder&
{
    m_path_parts.emplace_back(path_part);
    return *this;
}

auto QueryBuilder::AppendQueryParameter(
    std::string_view name,
    std::string_view value
) -> QueryBuilder&
{
    m_query_parameters.emplace_back(name, value);
    return *this;
}

auto QueryBuilder::SetFragment(
    std::string_view fragment
) -> QueryBuilder&
{
    m_fragment = fragment;
    return *this;
}

auto QueryBuilder::Build() -> std::string
{
    if(!m_scheme.empty())
    {
        m_query << m_scheme;
    }
    m_query << "://";
    if(!m_hostname.empty())
    {
        m_query << m_hostname;
    }
    if(m_port != 0)
    {
        m_query << ":" << m_port;
    }
    if(!m_path_parts.empty())
    {
        for(auto path_part : m_path_parts)
        {
            m_query << "/" << path_part;
        }
    }
    if(!m_query_parameters.empty())
    {
        bool first = true;

        for(const auto& [name, value] : m_query_parameters)
        {
            if(first)
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
    if(!m_fragment.empty())
    {
        m_query << "#" << m_fragment;
    }

    auto copy = m_query.str();
    reset();
    return copy;
}

auto QueryBuilder::reset() -> void
{
    m_query.clear();
    m_query.str("");
    m_scheme = std::string_view{};
    m_hostname = std::string_view{};
    m_port = 0;
    m_path_parts.clear();
    m_query_parameters.clear();
    m_fragment = std::string_view{};
}

} // lift
