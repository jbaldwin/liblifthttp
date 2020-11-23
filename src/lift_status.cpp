#include "lift/lift_status.hpp"

namespace lift
{
using namespace std::string_literals;

static const std::string lift_status_building              = "building"s;
static const std::string lift_status_executing             = "executing"s;
static const std::string lift_status_success               = "success"s;
static const std::string lift_status_connect_error         = "connect_error"s;
static const std::string lift_status_connect_dns_error     = "connect_dns_error"s;
static const std::string lift_status_connect_ssl_error     = "connect_ssl_error"s;
static const std::string lift_status_timeout               = "timeout"s;
static const std::string lift_status_response_empty        = "response_empty"s;
static const std::string lift_status_error                 = "error"s;
static const std::string lift_status_error_failed_to_start = "error_failed_to_start"s;
static const std::string lift_status_download_error        = "download_error"s;

auto to_string(lift_status status) -> const std::string&
{
    switch (status)
    {
        case lift_status::building:
            return lift_status_building;
        case lift_status::executing:
            return lift_status_executing;
        case lift_status::success:
            return lift_status_success;
        case lift_status::connect_error:
            return lift_status_connect_error;
        case lift_status::connect_dns_error:
            return lift_status_connect_dns_error;
        case lift_status::connect_ssl_error:
            return lift_status_connect_ssl_error;
        case lift_status::timeout:
            return lift_status_timeout;
        case lift_status::response_empty:
            return lift_status_response_empty;
        case lift_status::download_error:
            return lift_status_download_error;
        case lift_status::error_failed_to_start:
            return lift_status_error_failed_to_start;
        case lift_status::error:
        default:
            return lift_status_error;
    }
}

} // namespace lift
