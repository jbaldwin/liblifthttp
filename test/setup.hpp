#include <iostream>
#include <string>

#ifdef LIFT_LOCALHOST_TESTS
static inline const std::string service_ip_address = "127.0.0.1";
static inline const std::string nginx_hostname     = "localhost";
static inline const std::string haproxy_hostname   = "localhost";
#else
// Note that this IP Address is asigned by github actions and is subject to change.
static inline const std::string service_ip_address = "172.18.0.3";
static inline const std::string nginx_hostname     = "nginx";
static inline const std::string haproxy_hostname   = "haproxy";
#endif

static inline const uint16_t    nginx_port       = 80;
static inline const std::string nginx_port_str   = "80";
static inline const uint32_t    haproxy_port     = 3128;
static inline const std::string haproxy_port_str = "3128";
