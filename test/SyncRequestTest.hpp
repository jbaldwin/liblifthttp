#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("Synchronous 200")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/");
    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("Synchronous 404")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/not/here");
    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_404_NOT_FOUND);
}

TEST_CASE("Synchronous HEAD")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/");
    r->SetMethod(lift::http::Method::HEAD);
    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
    REQUIRE(r->GetResponseData().empty());
}

TEST_CASE("Synchrnous custom headers")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/");
    r->AddHeader("x-custom-header-1", "custom-value-1");

    for(const auto& header : r->GetRequestHeaders())
    {
        if(header.GetName() == "x-custom-header-1")
        {
            REQUIRE(header.GetValue() == "custom-value-1");
        }
    }
}

TEST_CASE("Multiple headers added")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/");
    r->AddHeader("Connection", "keep-alive");
    r->AddHeader("x-custom-header-1", "value1");
    r->AddHeader("x-custom-header-2", "value2");
    r->AddHeader("x-herp-derp", "merp");
    r->AddHeader("x-420", "blazeit");

    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);

    std::size_t count_found = 0;

    for(const auto& header : r->GetRequestHeaders())
    {
        if(header.GetName() == "Connection")
        {
            REQUIRE(header.GetValue() == "keep-alive");
            ++count_found;
        }
        else if(header.GetName() == "x-custom-header-1")
        {
            REQUIRE(header.GetValue() == "value1");
            ++count_found;
        }
        else if(header.GetName() == "x-custom-header-2")
        {
            REQUIRE(header.GetValue() == "value2");
            ++count_found;
        }
        else if(header.GetName() == "x-herp-derp")
        {
            REQUIRE(header.GetValue() == "merp");
            ++count_found;
        }
        else if(header.GetName() == "x-420")
        {
            REQUIRE(header.GetValue() == "blazeit");
            ++count_found;
        }
    }

    REQUIRE(count_found == 5);
}
