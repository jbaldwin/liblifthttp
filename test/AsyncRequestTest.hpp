#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Async 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::EventLoop ev{};
    auto& rp = ev.GetRequestPool();

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto r = rp.Produce(
            "http://localhost:80/",
            [](lift::RequestHandle rh, lift::Response response) -> void {
                REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
                REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
            },
            std::chrono::seconds{ 1 });

        ev.StartRequest(std::move(r));
    }

    while (ev.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    }
}

TEST_CASE("Async batch 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::EventLoop ev{};
    auto& rp = ev.GetRequestPool();

    std::vector<lift::RequestHandle> handles{};
    handles.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto r = rp.Produce(
            "http://localhost:80/",
            [](lift::RequestHandle rh, lift::Response response) -> void {
                REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
                REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
            },
            std::chrono::seconds{ 1 });

        handles.emplace_back(std::move(r));
    }

    ev.StartRequests(std::move(handles));

    while (ev.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
    }
}

TEST_CASE("Async POST request")
{
    lift::EventLoop ev{};
    auto& rp = ev.GetRequestPool();

    std::string data = "DATA DATA DATA!";

    auto request = rp.Produce(
        "http://localhost:80/",
        [&](lift::RequestHandle rh, lift::Response response) {
            REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
            REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_405_METHOD_NOT_ALLOWED);
        },
        std::chrono::seconds{ 60 });
    request->SetRequestData(data);
    request->SetMethod(lift::http::Method::POST);
    request->SetFollowRedirects(true);
    request->SetVersion(lift::http::Version::V1_1);
    //        request->AddHeader("Expect", "");

    ev.StartRequest(std::move(request));

    request = rp.Produce(
        "http://localhost:80/",
        [&](lift::RequestHandle rh, lift::Response response) {
            REQUIRE(response.GetCompletionStatus() == lift::RequestStatus::SUCCESS);
            REQUIRE(response.GetResponseStatusCode() == lift::http::StatusCode::HTTP_405_METHOD_NOT_ALLOWED);
        },
        std::chrono::seconds{ 60 });
    request->SetRequestData(data);
    request->SetMethod(lift::http::Method::POST);
    request->SetFollowRedirects(true);
    request->SetVersion(lift::http::Version::V1_1);
    // There was a bug where no expect header caused liblift to fail, test it explicitly
    request->AddHeader("Expect", "");

    ev.StartRequest(std::move(request));
}