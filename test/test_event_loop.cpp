#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("event_loop Start event loop, then stop and add a request.")
{
    lift::event_loop ev{};

    auto request = lift::request::make_unique(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::request>, lift::response response) {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        });

    REQUIRE(ev.start_request(std::move(request)));

    request = lift::request::make_unique(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::request>, lift::response response) {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        });

    // Adding requests after stopping should return false that they cannot be started.
    ev.stop();

    REQUIRE_FALSE(ev.start_request(std::move(request)));
}

TEST_CASE("event_loop Start event loop, then stop and add multiple requests.")
{
    lift::event_loop ev{};

    std::vector<lift::request_ptr> requests1;
    requests1.emplace_back(lift::request::make_unique(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::request>, lift::response response) {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        }));

    std::vector<lift::request_ptr> requests2;
    requests2.emplace_back(lift::request::make_unique(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/",
        std::chrono::seconds{60},
        [&](std::unique_ptr<lift::request>, lift::response response) {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
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
