#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("EventLoop Start event loop, then stop and add a request.")
{
    lift::EventLoop ev{};

    auto request = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        });

    REQUIRE(ev.StartRequest(std::move(request)));

    request = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        });

    // Adding requests after stopping should return false that they cannot be started.
    ev.Stop();

    REQUIRE_FALSE(ev.StartRequest(std::move(request)));
}

TEST_CASE("EventLoop Start event loop, then stop and add multiple requests.")
{
    lift::EventLoop ev{};

    std::vector<lift::RequestPtr> requests1;
    requests1.emplace_back(lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        }));

    std::vector<lift::RequestPtr> requests2;
    requests2.emplace_back(lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        }));

    REQUIRE(ev.StartRequests(std::move(requests1)));
    ev.Stop();
    REQUIRE_FALSE(ev.StartRequests(std::move(requests2)));
}

TEST_CASE("EventLoop Provide nullptr request")
{
    lift::EventLoop ev{};

    REQUIRE_FALSE(ev.StartRequest(nullptr));
}
