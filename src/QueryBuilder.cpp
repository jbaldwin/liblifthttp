#include "lift/QueryBuilder.h"
#include "lift/Escape.h"

namespace lift
{

QueryBuilder::QueryBuilder()
    : m_converter(),
      m_query(),
      m_scheme(),
      m_hostname(),
      m_port_str(),
      m_port_int(0),
      m_path_parts(),
      m_query_parameters(),
      m_fragment()
{
    m_query.reserve(2048);
}

auto QueryBuilder::SetScheme(
    StringView scheme
) -> QueryBuilder&
{
    m_scheme = scheme;
    return *this;
}

auto QueryBuilder::SetHostname(
    StringView hostname
) -> QueryBuilder&
{
    m_hostname = hostname;
    return *this;
}

auto QueryBuilder::SetPort(
    StringView port
) -> QueryBuilder&
{
    m_port_str = port;
    return *this;
}

auto QueryBuilder::SetPort(
    uint16_t port
) -> QueryBuilder&
{
    m_port_int = port;
    return *this;
}

auto QueryBuilder::AppendPathPart(
    StringView path_part
) -> QueryBuilder&
{
    m_path_parts.emplace_back(path_part);
    return *this;
}

auto QueryBuilder::AppendPathPart(
    StringView name,
    StringView value
) -> QueryBuilder&
{
    m_query_parameters.emplace_back(name, value);
    return *this;
}

auto QueryBuilder::SetFragment(
    StringView fragment
) -> QueryBuilder&
{
    m_fragment = fragment;
    return *this;
}

auto QueryBuilder::Build() -> std::string
{
    if(!m_scheme.empty())
    {
        m_query.append(m_scheme.data(), m_scheme.length());
    }
    m_query.append("://");
    if(!m_hostname.empty())
    {
        m_query.append(m_hostname.data(), m_hostname.length());
    }
    if(m_port_int != 0 || !m_port_str.empty())
    {
        m_query.append(":");
        if(m_port_int != 0)
        {
            m_converter.clear();
            m_converter.str("");
            m_converter << m_port_int;

            m_query.append(m_converter.str());
        }
        else
        {
            m_query.append(m_port_str.data(), m_port_str.length());
        }
    }
    if(!m_path_parts.empty())
    {
        for(auto path_part : m_path_parts)
        {
            m_query.append("/");
            m_query.append(path_part.data(), path_part.length());
        }
    }
    if(!m_query_parameters.empty())
    {
        bool first = true;

        for(auto& pair : m_query_parameters)
        {
            if(first)
            {
                m_query.append("?");
                first = false;
            }
            else
            {
                m_query.append("&");
            }
            auto escaped_data = lift::escape(pair.second);
            m_query.append(pair.first.data(), pair.first.length());
            m_query.append("=");
            m_query.append(escaped_data);
        }
    }
    if(!m_fragment.empty())
    {
        m_query.append("#");
        m_query.append(m_fragment.data(), m_fragment.length());
    }

    auto copy = m_query;
    reset();
    return copy;
}

auto QueryBuilder::reset() -> void
{
    m_query.clear();
    m_scheme = StringView();
    m_hostname = StringView();
    m_port_str = StringView();
    m_port_int = 0;
    m_path_parts.clear();
    m_query_parameters.clear();
    m_fragment = StringView();
}

} // lift
