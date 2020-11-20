#include "lift/lift_status.hpp"

namespace lift
{
using namespace std::string_literals;

static const std::string LIFT_STATUS_BUILDING              = "BUILDING"s;
static const std::string LIFT_STATUS_EXECUTING             = "EXECUTING"s;
static const std::string LIFT_STATUS_SUCCESS               = "SUCCESS"s;
static const std::string LIFT_STATUS_CONNECT_ERROR         = "CONNECT_ERROR"s;
static const std::string LIFT_STATUS_CONNECT_DNS_ERROR     = "CONNECT_DNS_ERROR"s;
static const std::string LIFT_STATUS_CONNECT_SSL_ERROR     = "CONNECT_SSL_ERROR"s;
static const std::string LIFT_STATUS_TIMEOUT               = "TIMEOUT"s;
static const std::string LIFT_STATUS_RESPONSE_EMPTY        = "RESPONSE_EMPTY"s;
static const std::string LIFT_STATUS_ERROR_FAILED_TO_START = "ERROR_FAILED_TO_START"s;
static const std::string LIFT_STATUS_ERROR                 = "ERROR"s;
static const std::string LIFT_STATUS_DOWNLOAD_ERROR        = "DOWNLOAD_ERROR"s;

auto to_string(LiftStatus status) -> const std::string&
{
    switch (status)
    {
        case LiftStatus::BUILDING:
            return LIFT_STATUS_BUILDING;
        case LiftStatus::EXECUTING:
            return LIFT_STATUS_EXECUTING;
        case LiftStatus::SUCCESS:
            return LIFT_STATUS_SUCCESS;
        case LiftStatus::CONNECT_ERROR:
            return LIFT_STATUS_CONNECT_ERROR;
        case LiftStatus::CONNECT_DNS_ERROR:
            return LIFT_STATUS_CONNECT_DNS_ERROR;
        case LiftStatus::CONNECT_SSL_ERROR:
            return LIFT_STATUS_CONNECT_SSL_ERROR;
        case LiftStatus::TIMEOUT:
            return LIFT_STATUS_TIMEOUT;
        case LiftStatus::RESPONSE_EMPTY:
            return LIFT_STATUS_RESPONSE_EMPTY;
        case LiftStatus::DOWNLOAD_ERROR:
            return LIFT_STATUS_DOWNLOAD_ERROR;
        case LiftStatus::ERROR_FAILED_TO_START:
            return LIFT_STATUS_ERROR_FAILED_TO_START;
        case LiftStatus::ERROR:
        default:
            return LIFT_STATUS_ERROR;
    }
}

} // namespace lift
