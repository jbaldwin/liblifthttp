#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

#include <iostream>

TEST_CASE("Proxy")
{
    // Need to do research but cannot add a port for the proxied request, leaving off for now.
    lift::Request request{"http://" + NGINX_HOSTNAME + "/"};
    request.Proxy(lift::ProxyType::HTTP, HAPROXY_HOSTNAME, HAPROXY_PORT);

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    for (const auto& header : response.Headers())
    {
        if (header.Name() == "server")
        {
            REQUIRE(header.Value() == "nginx/1.18.0");
        }
        else if (header.Name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.Value()});
            REQUIRE(content_length > 0);
        }
        else if (header.Name() == "content-type")
        {
            REQUIRE(header.Value() == "text/html");
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
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    for (const auto& header : response.Headers())
    {
        if (header.Name() == "server")
        {
            REQUIRE(header.Value() == "nginx/1.18.0");
        }
        else if (header.Name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.Value()});
            REQUIRE(content_length > 0);
        }
        else if (header.Name() == "content-type")
        {
            REQUIRE(header.Value() == "text/html");
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
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    for (const auto& header : response.Headers())
    {
        if (header.Name() == "server")
        {
            REQUIRE(header.Value() == "nginx/1.18.0");
        }
        else if (header.Name() == "content-length")
        {
            uint64_t content_length = std::stoul(std::string{header.Value()});
            REQUIRE(content_length > 0);
        }
        else if (header.Name() == "content-type")
        {
            REQUIRE(header.Value() == "text/html");
        }
    }
}