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