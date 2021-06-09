#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Async 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::client client{};

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::request>(
            "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{1});

        client.start_request(std::move(r), [](std::unique_ptr<lift::request> rh, lift::response response) -> void {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        });
    }

    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async batch 100 requests")
{
    constexpr std::size_t COUNT = 100;

    lift::client client{};

    std::vector<std::pair<std::unique_ptr<lift::request>, lift::request::async_callback_type>> handles{};
    handles.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i)
    {
        auto r = std::make_unique<lift::request>(
            "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{1});

        handles.emplace_back(
            std::make_pair(std::move(r), [](std::unique_ptr<lift::request>, lift::response response) -> void {
                REQUIRE(response.lift_status() == lift::lift_status::success);
                REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
            }));
    }

    client.start_requests(std::move(handles));

    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}

TEST_CASE("Async POST request")
{
    lift::client client{};

    std::string data = "DATA DATA DATA!";

    auto request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});
    request->data(data);
    request->method(lift::http::method::post);
    request->follow_redirects(true);
    request->version(lift::http::version::v1_1);
    //        request->header("Expect", "");

    client.start_request(std::move(request), [&](std::unique_ptr<lift::request>, lift::response response) {
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_405_method_not_allowed);
    });

    request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});
    request->data(data);
    request->method(lift::http::method::post);
    request->follow_redirects(true);
    request->version(lift::http::version::v1_1);
    // There was a bug where no expect header caused liblift to fail, test it explicitly
    request->header("Expect", "");

    client.start_request(std::move(request), [&](std::unique_ptr<lift::request>, lift::response response) {
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_405_method_not_allowed);
    });
}

TEST_CASE("Async POST request promise+future")
{
    lift::client client{};

    std::string data = "DATA DATA DATA!";

    auto request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});
    request->data(data);
    request->method(lift::http::method::post);
    request->follow_redirects(true);
    request->version(lift::http::version::v1_1);
    //        request->header("Expect", "");

    auto f1 = client.start_request(std::move(request));

    {
        auto [request_ptr, response] = f1.get();
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_405_method_not_allowed);
    }

    request = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});
    request->data(data);
    request->method(lift::http::method::post);
    request->follow_redirects(true);
    request->version(lift::http::version::v1_1);
    // There was a bug where no expect header caused liblift to fail, test it explicitly
    request->header("Expect", "");

    auto f2 = client.start_request(std::move(request));

    {
        auto [request_ptr, response] = f2.get();
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_405_method_not_allowed);
    }
}
