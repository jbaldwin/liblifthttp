#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "setup.hpp"

struct TestSetupInfo
{
    TestSetupInfo()
    {
        std::cout << "SERVICE_IP_ADDRESS = " << SERVICE_IP_ADDRESS << "\n";
        std::cout << "NGINX_HOSTNAME = "
                  << "http://" << NGINX_HOSTNAME << ":" << NGINX_PORT_STR << "/"
                  << "\n";
        std::cout << "HAPROXY_HOSTNAME = "
                  << "http://" << HAPROXY_HOSTNAME << ":" << HAPROXY_PORT_STR << "/"
                  << "\n";
    }
} test_setup_info_instance;
