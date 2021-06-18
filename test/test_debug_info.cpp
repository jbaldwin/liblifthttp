#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("request with debug info, verify callback is called at least once", "[debug_info]")
{
    lift::request request{"http://" + nginx_hostname + ":" + nginx_port_str + "/"};

    std::atomic<uint64_t> debug_info_called{0};

    auto on_debug_handler =
        [&debug_info_called](const lift::request& req, lift::debug_info_type type, std::string_view data) {
            debug_info_called.fetch_add(1, std::memory_order_relaxed);
        };

    request.debug_info_handler(on_debug_handler);

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    REQUIRE(debug_info_called.load(std::memory_order_relaxed) > 0);
}

TEST_CASE("debug request, reset request, debug requst again", "[debug_info]")
{
    lift::request request{"http://" + nginx_hostname + ":" + nginx_port_str + "/"};

    std::atomic<uint64_t> debug_info_called{0};

    auto on_debug_handler =
        [&debug_info_called](const lift::request& req, lift::debug_info_type type, std::string_view data) {
            debug_info_called.fetch_add(1, std::memory_order_relaxed);
        };

    request.debug_info_handler(on_debug_handler);

    const auto& response = request.perform();
    uint64_t    at       = debug_info_called.load(std::memory_order_relaxed);

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    REQUIRE(at > 0);

    const auto& response2 = request.perform();

    REQUIRE(response2.lift_status() == lift::lift_status::success);
    REQUIRE(response2.status_code() == lift::http::status_code::http_200_ok);
    // Using the same request should re-apply the debug function.
    REQUIRE(debug_info_called.load(std::memory_order_relaxed) > at);
    at = debug_info_called.load(std::memory_order_relaxed);

    lift::request request2{"http://" + nginx_hostname + ":" + nginx_port_str + "/"};
    const auto&   response3 = request2.perform();

    REQUIRE(response2.lift_status() == lift::lift_status::success);
    REQUIRE(response2.status_code() == lift::http::status_code::http_200_ok);
    // Using a new request shouldn't re-apply the debug function.
    REQUIRE(debug_info_called.load(std::memory_order_relaxed) == at);
}