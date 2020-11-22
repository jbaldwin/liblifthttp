#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

#include <iostream>

TEST_CASE("Transfer Progress synchronous")
{
    auto        request = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    std::size_t handler_called = 0;

    request->TransferProgressHandler(
        [&](const lift::Request& r, int64_t dltotal, int64_t dlnow, int64_t ultotal, int64_t ulnow) -> bool {
            handler_called++;
            return true; // continue the request
        });

    const auto& response = request->Perform();

    REQUIRE(handler_called > 0);
    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
}

TEST_CASE("Download <N> bytes test synchronous")
{
    auto request       = std::make_unique<lift::Request>("http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/");
    bool should_failed = true;

    static constexpr std::size_t BYTES_TO_DOWNLOAD = 5;

    request->TransferProgressHandler(
        [&](const lift::Request& r, int64_t dltotal, int64_t dlnow, int64_t, int64_t) -> bool {
            if (dlnow >= BYTES_TO_DOWNLOAD)
            {
                should_failed = true;
                return false;
            }
            else
            {
                if (dlnow >= dltotal)
                {
                    should_failed = false;
                }
                return true;
            }
        });

    const auto& response = request->Perform();

    // Its possible the test downloads the entire file before finishing, take the appropriate action.
    REQUIRE(response.LiftStatus() == ((should_failed) ? lift::LiftStatus::ERROR : lift::LiftStatus::SUCCESS));
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
}
