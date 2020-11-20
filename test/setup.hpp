#include <iostream>
#include <string>

#ifdef LIFT_LOCALHOST_TESTS
static inline const std::string SERVICE_IP_ADDRESS = "127.0.0.1";
static inline const std::string NGINX_HOSTNAME     = "localhost";
static inline const std::string HAPROXY_HOSTNAME   = "localhost";
#else
// Note that this IP Address is asigned by github actions and is subject to change.
static inline const std::string SERVICE_IP_ADDRESS = "172.18.0.3";
static inline const std::string NGINX_HOSTNAME     = "nginx";
static inline const std::string HAPROXY_HOSTNAME   = "haproxy";
#endif

static inline const uint16_t    NGINX_PORT       = 80;
static inline const std::string NGINX_PORT_STR   = "80";
static inline const uint32_t    HAPROXY_PORT     = 3128;
static inline const std::string HAPROXY_PORT_STR = "3128";
