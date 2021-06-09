#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Timesup single request")
{
    lift::client client{lift::client::options{.connect_timeout = std::chrono::seconds{1}}};

    auto r = lift::request::make_unique(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{25});

    client.start_request(std::move(r), [](std::unique_ptr<lift::request> rh, lift::response response) -> void {
        REQUIRE(response.lift_status() == lift::lift_status::timeout);
        REQUIRE(response.status_code() == lift::http::status_code::http_504_gateway_timeout);
        REQUIRE(response.total_time() == std::chrono::milliseconds{25});
        REQUIRE(response.num_connects() == 0);
        REQUIRE(response.num_redirects() == 0);
    });

    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Timesup two requests")
{
    lift::client client{lift::client::options{.connect_timeout = std::chrono::seconds{1}}};

    std::vector<std::pair<lift::request_ptr, lift::request::async_callback_type>> requests{};

    requests.push_back(std::make_pair(
        lift::request::make_unique(
            "http://www.reddit.com", // should be slow enough /shrug
            std::chrono::milliseconds{25}),
        [](std::unique_ptr<lift::request> rh, lift::response response) -> void {
            REQUIRE(response.lift_status() == lift::lift_status::timeout);
            REQUIRE(response.status_code() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.total_time() == std::chrono::milliseconds{25});
        }));

    requests.push_back(std::make_pair(
        lift::request::make_unique(
            "http://www.reddit.com", // should be slow enough /shrug
            std::chrono::milliseconds{50}),
        [](std::unique_ptr<lift::request> rh, lift::response response) -> void {
            REQUIRE(response.lift_status() == lift::lift_status::timeout);
            REQUIRE(response.status_code() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.total_time() == std::chrono::milliseconds{50});
        }));

    client.start_requests(std::move(requests));

    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}
