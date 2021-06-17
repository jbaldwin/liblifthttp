#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("request with debug info, verify callback is called at least once")
{
    lift::request request("http://" + nginx_hostname + ":" + nginx_port_str + "/");

    std::atomic<uint64_t> debug_info_called{0};

    request.debug_info_handler(
        [&debug_info_called](const lift::request& req, lift::debug_info_type type, std::string_view data) {
            ++debug_info_called;
        });

    const auto& response = request.perform();

    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    REQUIRE(debug_info_called > 0);
}