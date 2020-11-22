#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Async 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::event_loop ev{};

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::Request>(
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds{1},
            [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
                REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
                REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
            });

        ev.start_request(std::move(r));
    }

    while (!ev.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async batch 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::event_loop ev{};

    std::vector<std::unique_ptr<lift::Request>> handles{};
    handles.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::Request>(
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds{1},
            [](std::unique_ptr<lift::Request>, lift::Response response) -> void {
                REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
                REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
            });

        handles.emplace_back(std::move(r));
    }

    ev.start_requests(std::move(handles));

    while (!ev.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async POST request")
{
    lift::event_loop ev{};

    std::string data = "DATA DATA DATA!";

    auto request = std::make_unique<lift::Request>(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_405_method_not_allowed);
        });
    request->Data(data);
    request->Method(lift::http::method::post);
    request->FollowRedirects(true);
    request->Version(lift::http::version::v1_1);
    //        request->header("Expect", "");

    ev.start_request(std::move(request));

    request = std::make_unique<lift::Request>(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_405_method_not_allowed);
        });
    request->Data(data);
    request->Method(lift::http::method::post);
    request->FollowRedirects(true);
    request->Version(lift::http::version::v1_1);
    // There was a bug where no expect header caused liblift to fail, test it explicitly
    request->Header("Expect", "");

    ev.start_request(std::move(request));
}
