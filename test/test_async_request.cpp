#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Async 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::EventLoop ev{};

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::Request>(
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds{1},
            [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
                REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
                REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
            });

        ev.StartRequest(std::move(r));
    }

    while (ev.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async batch 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::EventLoop ev{};

    std::vector<std::unique_ptr<lift::Request>> handles{};
    handles.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::Request>(
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds{1},
            [](std::unique_ptr<lift::Request>, lift::Response response) -> void {
                REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
                REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
            });

        handles.emplace_back(std::move(r));
    }

    ev.StartRequests(std::move(handles));

    while (ev.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async POST request")
{
    lift::EventLoop ev{};

    std::string data = "DATA DATA DATA!";

    auto request = std::make_unique<lift::Request>(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_405_METHOD_NOT_ALLOWED);
        });
    request->Data(data);
    request->Method(lift::http::Method::POST);
    request->FollowRedirects(true);
    request->Version(lift::http::Version::V1_1);
    //        request->AddHeader("Expect", "");

    ev.StartRequest(std::move(request));

    request = std::make_unique<lift::Request>(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_405_METHOD_NOT_ALLOWED);
        });
    request->Data(data);
    request->Method(lift::http::Method::POST);
    request->FollowRedirects(true);
    request->Version(lift::http::Version::V1_1);
    // There was a bug where no expect header caused liblift to fail, test it explicitly
    request->Header("Expect", "");

    ev.StartRequest(std::move(request));
}
