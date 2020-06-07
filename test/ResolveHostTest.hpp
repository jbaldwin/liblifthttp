#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("ResolveHost synchronous perform")
{
    lift::ResolveHost rhost {
        "testhostname",
        80,
        SERVICE_IP_ADDRESS
    };

    REQUIRE(rhost.Host() == "testhostname");
    REQUIRE(rhost.Port() == 80);
    REQUIRE(rhost.IpAddr() == SERVICE_IP_ADDRESS);

    lift::Request request {
        "http://testhostname:80/"
    };

    request.ResolveHost(std::move(rhost));

    auto response = request.Perform();
    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("EventLoop ResolveHost")
{
    std::vector<lift::ResolveHost> rhosts {
        lift::ResolveHost { "testhostname", 80, SERVICE_IP_ADDRESS },
        lift::ResolveHost { "herpderp.com", 80, SERVICE_IP_ADDRESS }
    };

    lift::EventLoop ev {
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::move(rhosts)
    };

    auto on_complete = [&](lift::RequestPtr, lift::Response response) -> void {
        REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
        REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    };

    std::vector<lift::RequestPtr> requests;
    requests.emplace_back(
        lift::Request::make_unique(
            "testhostname",
            std::chrono::seconds { 60 },
            on_complete));
    requests.emplace_back(
        lift::Request::make_unique(
            "herpderp.com",
            std::chrono::seconds { 60 },
            on_complete));

    REQUIRE(ev.StartRequests(std::move(requests)));
}