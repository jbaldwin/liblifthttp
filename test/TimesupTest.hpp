#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Timesup single request")
{
    lift::EventLoop ev{std::nullopt, std::nullopt, {std::chrono::seconds{1}}};

    auto r = lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{25},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_504_GATEWAY_TIMEOUT);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{25});
            REQUIRE(response.NumConnects() == 0);
            REQUIRE(response.NumRedirects() == 0);
        });

    ev.StartRequest(std::move(r));

    while (ev.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Timesup two requests")
{
    lift::EventLoop ev{std::nullopt, std::nullopt, {std::chrono::seconds{1}}};

    std::vector<lift::RequestPtr> requests{};

    requests.push_back(lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{25},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_504_GATEWAY_TIMEOUT);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{25});
        }));

    requests.push_back(lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{50},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_504_GATEWAY_TIMEOUT);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{50});
        }));

    ev.StartRequests(std::move(requests));

    while (ev.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}
