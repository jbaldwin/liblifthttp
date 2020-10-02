#pragma once

#include "catch.hpp"

#include <chrono>
#include <lift/Lift.hpp>

TEST_CASE("Synchronous 200")
{
    lift::Request request("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    const auto&   response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("Synchronous 404")
{
    lift::Request request("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/not/here");
    const auto&   response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_404_NOT_FOUND);
}

TEST_CASE("Synchronous HEAD")
{
    lift::Request request("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    request.Method(lift::http::Method::HEAD);
    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    REQUIRE(response.Data().empty());
}

TEST_CASE("Synchronous custom headers")
{
    lift::Request request("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    request.Header("x-custom-header-1", "custom-value-1");

    for (const auto& header : request.Headers())
    {
        if (header.Name() == "x-custom-header-1")
        {
            REQUIRE(header.Value() == "custom-value-1");
        }
    }
}

TEST_CASE("Multiple headers added")
{
    lift::Request request("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    request.Header("Connection", "keep-alive");
    request.Header("x-custom-header-1", "value1");
    request.Header("x-custom-header-2", "value2");
    request.Header("x-herp-derp", "merp");
    request.Header("x-420", "blazeit");

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);

    std::size_t count_found = 0;

    for (const auto& header : request.Headers())
    {
        if (header.Name() == "Connection")
        {
            REQUIRE(header.Value() == "keep-alive");
            ++count_found;
        }
        else if (header.Name() == "x-custom-header-1")
        {
            REQUIRE(header.Value() == "value1");
            ++count_found;
        }
        else if (header.Name() == "x-custom-header-2")
        {
            REQUIRE(header.Value() == "value2");
            ++count_found;
        }
        else if (header.Name() == "x-herp-derp")
        {
            REQUIRE(header.Value() == "merp");
            ++count_found;
        }
        else if (header.Name() == "x-420")
        {
            REQUIRE(header.Value() == "blazeit");
            ++count_found;
        }
    }

    REQUIRE(count_found == 5);
}

TEST_CASE("Happy Eyeballs Test")
{
    using namespace std::chrono_literals;
    lift::Request request{"http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/"};
    request.HappyEyeballsTimeout(0ms);

    const auto& response = request.Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("SSL functions")
{
    // TODO, some of these require files.
}