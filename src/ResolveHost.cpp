#include "lift/ResolveHost.hpp"

namespace lift {

ResolveHost::ResolveHost(
    std::string resolve_host,
    uint16_t resolve_port,
    std::string resolved_ip_addr)
    : m_resolve_host(std::move(resolve_host))
    , m_resolve_port(resolve_port)
    , m_resolved_ip_addr(std::move(resolved_ip_addr))
{
    constexpr size_t RESERVE_BYTES_PORT = 16; // ports are 2^16 which is maximum 5 bytes, this should do.

    m_curl_formatted.reserve(m_resolve_host.length() + m_resolved_ip_addr.length() + RESERVE_BYTES_PORT);

    m_curl_formatted.append(m_resolve_host);
    m_curl_formatted.append(":");
    m_curl_formatted.append(std::to_string(m_resolve_port));
    m_curl_formatted.append(":");
    m_curl_formatted.append(m_resolved_ip_addr);
}

} // namespace lift
