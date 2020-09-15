#define CATCH_CONFIG_MAIN

#include <iostream>
#include <string>

#ifdef LIFT_LOCALHOST_TESTS
static const std::string SERVICE_IP_ADDRESS = "127.0.0.1";
static const std::string NGINX_HOSTNAME = "localhost";
static const std::string HAPROXY_HOSTNAME = "localhost";
#else
// Note that this IP Address is asigned by github actions and is subject to change.
static const std::string SERVICE_IP_ADDRESS = "172.18.0.3";
static const std::string NGINX_HOSTNAME = "nginx";
static const std::string HAPROXY_HOSTNAME = "haproxy";
#endif

static const uint16_t NGINX_PORT = 80;
static const std::string NGINX_PORT_STR = "80";
static const uint32_t HAPROXY_PORT = 3128;
static const std::string HAPROXY_PORT_STR = "3128";

struct TestSetupInfo {
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

#include "catch.hpp"

#include <lift/Lift.hpp>

#include "AsyncRequestTest.hpp"
#include "EscapeTest.hpp"
#include "EventLoopTest.hpp"
#include "HeaderTest.hpp"
#include "HttpTest.hpp"
#include "MimeFieldTest.hpp"
#include "ProxyTest.hpp"
#include "QueryBuilderTest.hpp"
#include "ResolveHostTest.hpp"
#include "ShareTest.hpp"
#include "SyncRequestTest.hpp"
#include "TimesupTest.hpp"
#include "TransferProgressRequest.hpp"
#include "UserDataRequestTest.hpp"
