#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <chrono>
#include <vector>

TEST_CASE("Timesup single request")
{
    lift::client client{lift::client::options{.connect_timeout = std::chrono::seconds{1}}};

    auto r = std::make_unique<lift::request>(
        "http://www.reddit.com", // should be slow enough /shrug
        std::chrono::milliseconds{5});

    client.start_request(
        std::move(r),
        [](std::unique_ptr<lift::request> rh, lift::response response) -> void
        {
            REQUIRE(response.lift_status() == lift::lift_status::timeout);
            REQUIRE(response.status_code() == lift::http::status_code::http_504_gateway_timeout);
            REQUIRE(response.total_time() == std::chrono::milliseconds{5});
            REQUIRE(response.num_connects() == 0);
            REQUIRE(response.num_redirects() == 0);
        });
}

TEST_CASE("Timesup two requests")
{
    lift::client client{lift::client::options{.connect_timeout = std::chrono::seconds{1}}};

    std::vector<lift::request_ptr> requests{};

    auto callback = [](std::unique_ptr<lift::request> rh, lift::response response) -> void
    {
        REQUIRE(response.lift_status() == lift::lift_status::timeout);
        REQUIRE(response.status_code() == lift::http::status_code::http_504_gateway_timeout);
        REQUIRE(
            (response.total_time() == std::chrono::milliseconds{5} ||
             response.total_time() == std::chrono::milliseconds{10}));
    };

    // should be slow enough /shrug
    requests.push_back(std::make_unique<lift::request>("http://www.reddit.com", std::chrono::milliseconds{5}));
    requests.push_back(std::make_unique<lift::request>("http://www.reddit.com", std::chrono::milliseconds{10}));

    client.start_requests(std::move(requests), callback);
}
