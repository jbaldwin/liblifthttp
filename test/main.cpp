#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "setup.hpp"

struct TestSetupInfo
{
    TestSetupInfo()
    {
        std::cout << "service_ip_address = " << service_ip_address << "\n";
        std::cout << "nginx_hostname = "
                  << "http://" << nginx_hostname << ":" << nginx_port_str << "/"
                  << "\n";
        std::cout << "haproxy_hostname = "
                  << "http://" << haproxy_hostname << ":" << haproxy_port_str << "/"
                  << "\n";
    }
} test_setup_info_instance;
