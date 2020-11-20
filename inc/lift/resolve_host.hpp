#pragma once

#include <string>

namespace lift
{
class Executor;

class ResolveHost
{
    /// For getCurlFormattedResolveHost().
    friend Executor;

public:
    /**
     * Sets a host and port combination to DNS resolve to the given IP Address.
     * @param resolve_host The host to resolve.
     * @param resolve_port The port to resolve.
     * @param resolved_ip_addr The IP Address that the host + port combination should resolve to.
     */
    ResolveHost(std::string resolve_host, uint16_t resolve_port, std::string resolved_ip_addr);
    ~ResolveHost() = default;

    ResolveHost(const ResolveHost&)     = default;
    ResolveHost(ResolveHost&&) noexcept = default;
    auto operator=(const ResolveHost&) -> ResolveHost& = default;
    auto operator=(ResolveHost&&) noexcept -> ResolveHost& = default;

    /**
     * @return Gets the given host that should be resolved.
     */
    [[nodiscard]] auto Host() const noexcept -> const std::string& { return m_resolve_host; }

    /**
     * @return Gets the given port that should be resolved.
     */
    [[nodiscard]] auto Port() const noexcept -> uint16_t { return m_resolve_port; }

    /**
     * @return Gets the given IP Address that is being resolved to.
     */
    [[nodiscard]] auto IpAddr() const noexcept -> const std::string& { return m_resolved_ip_addr; }

private:
    /// The given input resolve host.
    std::string m_resolve_host{};
    /// The given input resolve port.
    uint16_t m_resolve_port{};
    /// The ip address to resolve the given host:port pair.
    std::string m_resolved_ip_addr{};

    /// A curl formatted version of the host + port + resolved ip address.
    std::string m_curl_formatted{};

    /**
     * @return Gets the "host:port:ipaddress" curl formatted resolve host.
     */
    [[nodiscard]] auto getCurlFormattedResolveHost() const noexcept -> const std::string& { return m_curl_formatted; }
};

} // namespace lift
