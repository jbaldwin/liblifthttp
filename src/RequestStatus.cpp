#include "lift/RequestStatus.h"

namespace lift {

using namespace std::string_literals;

static const std::string REQUEST_STATUS_BUILDING = "BUILDING"s;
static const std::string REQUEST_STATUS_EXECUTING = "EXECUTING"s;
static const std::string REQUEST_STATUS_SUCCESS = "SUCCESS"s;
static const std::string REQUEST_STATUS_CONNECT_ERROR = "CONNECT_ERROR"s;
static const std::string REQUEST_STATUS_CONNECT_DNS_ERROR = "CONNECT_DNS_ERROR"s;
static const std::string REQUEST_STATUS_CONNECT_SSL_ERROR = "CONNECT_SSL_ERROR"s;
static const std::string REQUEST_STATUS_TIMEOUT = "TIMEOUT"s;
static const std::string REQUEST_STATUS_RESPONSE_EMPTY = "RESPONSE_EMPTY"s;
static const std::string REQUEST_STATUS_ERROR_FAILED_TO_START = "ERROR_FAILED_TO_START"s;
static const std::string REQUEST_STATUS_ERROR = "ERROR"s;
static const std::string REQUEST_STATUS_DOWNLOAD_ERROR = "DOWNLOAD_ERROR"s;

auto to_string(RequestStatus request_status) -> const std::string&
{
    switch (request_status) {
    case RequestStatus::BUILDING:
        return REQUEST_STATUS_BUILDING;
    case RequestStatus::EXECUTING:
        return REQUEST_STATUS_EXECUTING;
    case RequestStatus::SUCCESS:
        return REQUEST_STATUS_SUCCESS;
    case RequestStatus::CONNECT_ERROR:
        return REQUEST_STATUS_CONNECT_ERROR;
    case RequestStatus::CONNECT_DNS_ERROR:
        return REQUEST_STATUS_CONNECT_DNS_ERROR;
    case RequestStatus::CONNECT_SSL_ERROR:
        return REQUEST_STATUS_CONNECT_SSL_ERROR;
    case RequestStatus::TIMEOUT:
        return REQUEST_STATUS_TIMEOUT;
    case RequestStatus::RESPONSE_EMPTY:
        return REQUEST_STATUS_RESPONSE_EMPTY;
    case RequestStatus::DOWNLOAD_ERROR:
        return REQUEST_STATUS_DOWNLOAD_ERROR;
    case RequestStatus::ERROR_FAILED_TO_START:
        return REQUEST_STATUS_ERROR_FAILED_TO_START;
    case RequestStatus::ERROR:
    default:
        return REQUEST_STATUS_ERROR;
    }
}

} // lift
