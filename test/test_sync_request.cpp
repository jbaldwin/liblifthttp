#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>

TEST_CASE("Synchronous 200")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/");
    const auto&   response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
}

TEST_CASE("Synchronous 404")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/not/here");
    const auto&   response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_404_not_found);
}

TEST_CASE("Synchronous HEAD")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/");
    request.method(lift::http::method::head);
    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    REQUIRE(response.data().empty());
}

TEST_CASE("Synchronous custom headers")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/");
    request.header("x-custom-header-1", "custom-value-1");

    for (const auto& header : request.headers())
    {
        if (header.name() == "x-custom-header-1")
        {
            REQUIRE(header.value() == "custom-value-1");
        }
    }
}

TEST_CASE("Multiple headers added")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/");
    request.header("Connection", "keep-alive");
    request.header("x-custom-header-1", "value1");
    request.header("x-custom-header-2", "value2");
    request.header("x-herp-derp", "merp");
    request.header("x-420", "blazeit");

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);

    std::size_t count_found = 0;

    for (const auto& header : request.headers())
    {
        if (header.name() == "Connection")
        {
            REQUIRE(header.value() == "keep-alive");
            ++count_found;
        }
        else if (header.name() == "x-custom-header-1")
        {
            REQUIRE(header.value() == "value1");
            ++count_found;
        }
        else if (header.name() == "x-custom-header-2")
        {
            REQUIRE(header.value() == "value2");
            ++count_found;
        }
        else if (header.name() == "x-herp-derp")
        {
            REQUIRE(header.value() == "merp");
            ++count_found;
        }
        else if (header.name() == "x-420")
        {
            REQUIRE(header.value() == "blazeit");
            ++count_found;
        }
    }

    REQUIRE(count_found == 5);
}

TEST_CASE("Happy Eyeballs Test")
{
    using namespace std::chrono_literals;
    lift::request request{"http://" + nginx_hostname + ":" + nginx_port_str + "/"};
    request.happy_eyeballs_timeout(0ms);

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
}

TEST_CASE("SSL functions")
{
    // TODO, some of these require files.
}
