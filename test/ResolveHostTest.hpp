#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("ResolveHost synchronous perform")
{
    lift::ResolveHost rhost{"testhostname", NGINX_PORT, SERVICE_IP_ADDRESS};

    REQUIRE(rhost.Host() == "testhostname");
    REQUIRE(rhost.Port() == NGINX_PORT);
    REQUIRE(rhost.IpAddr() == SERVICE_IP_ADDRESS);

    lift::Request request{"http://testhostname:" + NGINX_PORT_STR + "/"};

    request.ResolveHost(std::move(rhost));

    auto response = request.Perform();
    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
}

TEST_CASE("EventLoop ResolveHost")
{
    std::vector<lift::ResolveHost> rhosts{
        lift::ResolveHost{"testhostname", NGINX_PORT, SERVICE_IP_ADDRESS},
        lift::ResolveHost{"herpderp.com", NGINX_PORT, SERVICE_IP_ADDRESS}};

    lift::EventLoop ev{std::nullopt, std::nullopt, std::nullopt, std::move(rhosts)};

    auto on_complete = [&](lift::RequestPtr, lift::Response response) -> void {
        REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
        REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    };

    std::vector<lift::RequestPtr> requests;
    requests.emplace_back(
        lift::Request::make_unique("testhostname:" + NGINX_PORT_STR, std::chrono::seconds{60}, on_complete));
    requests.emplace_back(
        lift::Request::make_unique("herpderp.com:" + NGINX_PORT_STR, std::chrono::seconds{60}, on_complete));

    REQUIRE(ev.StartRequests(std::move(requests)));
}