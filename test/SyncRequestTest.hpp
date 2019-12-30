#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("Simple synchronous 200", "[200 OK]")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/");
    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("Simple synchronous 404", "[404 Not Found]")
{
    lift::RequestPool rp{};
    auto r = rp.Produce("http://localhost:80/nothere");
    r->Perform();

    REQUIRE(r->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(r->GetResponseStatusCode() == lift::http::StatusCode::HTTP_404_NOT_FOUND);
}