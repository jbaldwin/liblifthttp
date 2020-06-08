#define CATCH_CONFIG_MAIN

#include <iostream>
#include <string>

#ifdef LIFT_LOCALHOST_TESTS
static const std::string SERVICE_IP_ADDRESS = "127.0.0.1";
static const std::string NGINX_HOSTNAME = "localhost";
#else
// Note that this IP Address is asigned by github actions and is subject to change.
static const std::string SERVICE_IP_ADDRESS = "172.18.0.3";
static const std::string NGINX_HOSTNAME = "nginx";
#endif

static const uint16_t NGINX_PORT = 80;
static const std::string NGINX_PORT_STR = "80";

struct TestSetupInfo {
    TestSetupInfo()
    {
        std::cout << "SERVICE_IP_ADDRESS = " << SERVICE_IP_ADDRESS << "\n";
        std::cout << "NGINX_HOSTNAME = "
                  << "http://" << NGINX_HOSTNAME << ":" << NGINX_PORT_STR << "/"
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
#include "QueryBuilderTest.hpp"
#include "ResolveHostTest.hpp"
#include "SyncRequestTest.hpp"
#include "TimesupTest.hpp"
#include "TransferProgressRequest.hpp"
#include "UserDataRequestTest.hpp"