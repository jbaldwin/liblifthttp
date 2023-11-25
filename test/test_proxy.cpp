#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <iostream>

TEST_CASE("proxy")
{
    // Need to do research but cannot add a port for the proxied request, leaving off for now.
    lift::request request{"http://" + nginx_hostname + "/"};
    request.proxy(lift::proxy_type::http, haproxy_hostname, haproxy_port);

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.24.0");
        }
        else if (header.name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.value()});
            REQUIRE(content_length > 0);
        }
        else if (header.name() == "content-type")
        {
            REQUIRE(header.value() == "text/html");
        }
    }
}

TEST_CASE("proxy Basic Auth")
{
    lift::request request{"http://" + nginx_hostname + "/"};
    request.proxy(
        lift::proxy_type::http,
        haproxy_hostname,
        haproxy_port,
        "guest",
        "guestpassword",
        std::vector{lift::http_auth_type::basic});

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.24.0");
        }
        else if (header.name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.value()});
            REQUIRE(content_length > 0);
        }
        else if (header.name() == "content-type")
        {
            REQUIRE(header.value() == "text/html");
        }
    }
}

TEST_CASE("proxy Any Auth")
{
    lift::request request{"http://" + nginx_hostname + "/"};
    request.proxy(
        lift::proxy_type::http,
        haproxy_hostname,
        haproxy_port,
        "guest",
        "guestpassword",
        std::vector{lift::http_auth_type::any_safe});

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.24.0");
        }
        else if (header.name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.value()});
            REQUIRE(content_length > 0);
        }
        else if (header.name() == "content-type")
        {
            REQUIRE(header.value() == "text/html");
        }
    }
}
