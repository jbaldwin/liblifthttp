#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("Synchronous 200")
{
    lift::RequestPool rp{};
    auto request = rp.Produce("http://localhost:80/");
    const auto& response = request->Perform();

    REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("Synchronous 404")
{
    lift::RequestPool rp{};
    auto request = rp.Produce("http://localhost:80/not/here");
    const auto& response = request->Perform();

    REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_404_NOT_FOUND);
}

TEST_CASE("Synchronous HEAD")
{
    lift::RequestPool rp{};
    auto request = rp.Produce("http://localhost:80/");
    request->SetMethod(lift::http::Method::HEAD);
    const auto& response = request->Perform();

    REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
    REQUIRE(response.GetResponseData().empty());
}

TEST_CASE("Synchronous custom headers")
{
    lift::RequestPool rp{};
    auto request = rp.Produce("http://localhost:80/");
    request->AddHeader("x-custom-header-1", "custom-value-1");

    for (const auto& header : request->GetRequestHeaders()) {
        if (header.GetName() == "x-custom-header-1") {
            REQUIRE(header.GetValue() == "custom-value-1");
        }
    }
}

TEST_CASE("Multiple headers added")
{
    lift::RequestPool rp{};
    auto request = rp.Produce("http://localhost:80/");
    request->AddHeader("Connection", "keep-alive");
    request->AddHeader("x-custom-header-1", "value1");
    request->AddHeader("x-custom-header-2", "value2");
    request->AddHeader("x-herp-derp", "merp");
    request->AddHeader("x-420", "blazeit");

    const auto& response = request->Perform();

    REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);

    std::size_t count_found = 0;

    for (const auto& header : request->GetRequestHeaders()) {
        if (header.GetName() == "Connection") {
            REQUIRE(header.GetValue() == "keep-alive");
            ++count_found;
        } else if (header.GetName() == "x-custom-header-1") {
            REQUIRE(header.GetValue() == "value1");
            ++count_found;
        } else if (header.GetName() == "x-custom-header-2") {
            REQUIRE(header.GetValue() == "value2");
            ++count_found;
        } else if (header.GetName() == "x-herp-derp") {
            REQUIRE(header.GetValue() == "merp");
            ++count_found;
        } else if (header.GetName() == "x-420") {
            REQUIRE(header.GetValue() == "blazeit");
            ++count_found;
        }
    }

    REQUIRE(count_found == 5);
}
