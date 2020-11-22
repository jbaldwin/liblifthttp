#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("event_loop Start event loop, then stop and add a request.")
{
    lift::event_loop ev{};

    auto request = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
        });

    REQUIRE(ev.start_request(std::move(request)));

    request = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
        });

    // Adding requests after stopping should return false that they cannot be started.
    ev.stop();

    REQUIRE_FALSE(ev.start_request(std::move(request)));
}

TEST_CASE("event_loop Start event loop, then stop and add multiple requests.")
{
    lift::event_loop ev{};

    std::vector<lift::RequestPtr> requests1;
    requests1.emplace_back(lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
        }));

    std::vector<lift::RequestPtr> requests2;
    requests2.emplace_back(lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
        }));

    REQUIRE(ev.start_requests(std::move(requests1)));
    ev.stop();
    REQUIRE_FALSE(ev.start_requests(std::move(requests2)));
}

TEST_CASE("event_loop Provide nullptr request")
{
    lift::event_loop ev{};

    REQUIRE_FALSE(ev.start_request(nullptr));
}
