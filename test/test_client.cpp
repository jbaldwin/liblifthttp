#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("client Start event loop, then stop and add a request.")
{
    lift::client client{};

    auto request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});

    REQUIRE_NOTHROW(client.start_request(
        std::move(request),
        [&](std::unique_ptr<lift::request>, lift::response response)
        {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        }));

    request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});

    // Adding requests after stopping should return false that they cannot be started.
    client.stop();

    REQUIRE_NOTHROW(client.start_request(
        std::move(request),
        [&](std::unique_ptr<lift::request>, lift::response response)
        {
            REQUIRE(response.lift_status() == lift::lift_status::error_failed_to_start);
            REQUIRE(response.status_code() == lift::http::status_code::http_500_internal_server_error);
        }));
}

TEST_CASE("client Start event loop, then stop and add multiple requests futures.")
{
    lift::client client{};

    std::vector<lift::request_ptr> requests1;
    requests1.emplace_back(std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}));

    std::vector<lift::request_ptr> requests2;
    requests2.emplace_back(std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}));

    auto futures1 = client.start_requests(std::move(requests1));
    client.stop();
    auto futures2 = client.start_requests(std::move(requests2));

    for (auto& f : futures1)
    {
        auto [req, rep] = f.get();
        REQUIRE(rep.lift_status() == lift::lift_status::success);
        REQUIRE(rep.status_code() == lift::http::status_code::http_200_ok);
    }

    for (auto& f : futures2)
    {
        auto [req, rep] = f.get();
        REQUIRE(rep.lift_status() == lift::lift_status::error_failed_to_start);
        REQUIRE(rep.status_code() == lift::http::status_code::http_500_internal_server_error);
    }
}

TEST_CASE("client Start event loop, then stop and add multiple requests callback.")
{
    lift::client client{};

    auto callback1 = [&](std::unique_ptr<lift::request>, lift::response response)
    {
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    };

    auto callback2 = [&](std::unique_ptr<lift::request>, lift::response response)
    {
        REQUIRE(response.lift_status() == lift::lift_status::error_failed_to_start);
        REQUIRE(response.status_code() == lift::http::status_code::http_500_internal_server_error);
    };

    std::vector<lift::request_ptr> requests1;
    requests1.emplace_back(std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}));

    std::vector<lift::request_ptr> requests2;
    requests2.emplace_back(std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}));

    client.start_requests(std::move(requests1), callback1);
    client.stop();
    client.start_requests(std::move(requests2), callback2);
}

TEST_CASE("client Provide nullptr request")
{
    lift::client client{};

    REQUIRE_THROWS(client.start_request(nullptr, nullptr));
}

TEST_CASE("client Provide nullptr lambda functor")
{
    lift::client client{};

    auto request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});

    REQUIRE_THROWS(client.start_request(std::move(request), nullptr));
}

TEST_CASE("client Provide nullptr lambda functor for batch requests")
{
    lift::client client{};

    std::vector<lift::request_ptr> requests{};
    requests.emplace_back(std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}));

    REQUIRE_THROWS(client.start_requests(std::move(requests), nullptr));
}