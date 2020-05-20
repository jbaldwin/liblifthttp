#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("Synchronous 200")
{
    auto request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":80/");
    const auto& response = request->Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("Synchronous 404")
{
    auto request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":80/not/here");
    const auto& response = request->Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_404_NOT_FOUND);
}

TEST_CASE("Synchronous HEAD")
{
    auto request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":80/");
    request->Method(lift::http::Method::HEAD);
    const auto& response = request->Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    REQUIRE(response.Data().empty());
}

TEST_CASE("Synchronous custom headers")
{
    auto request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":80/");
    request->Header("x-custom-header-1", "custom-value-1");

    for (const auto& header : request->Headers()) {
        if (header.Name() == "x-custom-header-1") {
            REQUIRE(header.Value() == "custom-value-1");
        }
    }
}

TEST_CASE("Multiple headers added")
{
    auto request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":80/");
    request->Header("Connection", "keep-alive");
    request->Header("x-custom-header-1", "value1");
    request->Header("x-custom-header-2", "value2");
    request->Header("x-herp-derp", "merp");
    request->Header("x-420", "blazeit");

    const auto& response = request->Perform();

    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);

    std::size_t count_found = 0;

    for (const auto& header : request->Headers()) {
        if (header.Name() == "Connection") {
            REQUIRE(header.Value() == "keep-alive");
            ++count_found;
        } else if (header.Name() == "x-custom-header-1") {
            REQUIRE(header.Value() == "value1");
            ++count_found;
        } else if (header.Name() == "x-custom-header-2") {
            REQUIRE(header.Value() == "value2");
            ++count_found;
        } else if (header.Name() == "x-herp-derp") {
            REQUIRE(header.Value() == "merp");
            ++count_found;
        } else if (header.Name() == "x-420") {
            REQUIRE(header.Value() == "blazeit");
            ++count_found;
        }
    }

    REQUIRE(count_found == 5);
}
