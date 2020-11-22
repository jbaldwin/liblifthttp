#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <iostream>

TEST_CASE("Proxy")
{
    // Need to do research but cannot add a port for the proxied request, leaving off for now.
    lift::Request request{"http://" + NGINX_HOSTNAME + "/"};
    request.Proxy(lift::ProxyType::HTTP, HAPROXY_HOSTNAME, HAPROXY_PORT);

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.Headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.18.0");
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

TEST_CASE("Proxy Basic Auth")
{
    lift::Request request{"http://" + NGINX_HOSTNAME + "/"};
    request.Proxy(
        lift::ProxyType::HTTP,
        HAPROXY_HOSTNAME,
        HAPROXY_PORT,
        "guest",
        "guestpassword",
        std::vector{lift::HttpAuthType::BASIC});

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.Headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.18.0");
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

TEST_CASE("Proxy Any Auth")
{
    lift::Request request{"http://" + NGINX_HOSTNAME + "/"};
    request.Proxy(
        lift::ProxyType::HTTP,
        HAPROXY_HOSTNAME,
        HAPROXY_PORT,
        "guest",
        "guestpassword",
        std::vector{lift::HttpAuthType::ANY});

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
    for (const auto& header : response.Headers())
    {
        if (header.name() == "server")
        {
            REQUIRE(header.value() == "nginx/1.18.0");
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
