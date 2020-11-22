#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Timesup single request")
{
    lift::event_loop ev{lift::event_loop::options{.connect_timeout = std::chrono::seconds{1}}};

    auto r = lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{25},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{25});
            REQUIRE(response.NumConnects() == 0);
            REQUIRE(response.NumRedirects() == 0);
        });

    ev.start_request(std::move(r));

    while (!ev.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Timesup two requests")
{
    lift::event_loop ev{lift::event_loop::options{.connect_timeout = std::chrono::seconds{1}}};

    std::vector<lift::RequestPtr> requests{};

    requests.push_back(lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{25},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{25});
        }));

    requests.push_back(lift::Request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{50},
        [](std::unique_ptr<lift::Request> rh, lift::Response response) -> void {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::TIMEOUT);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.TotalTime() == std::chrono::milliseconds{50});
        }));

    ev.start_requests(std::move(requests));

    while (!ev.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}
