#pragma once

#include <string>

namespace lift {

enum class RequestStatus {
    BUILDING, ///< The request is under construction.
    EXECUTING, ///< The request is being executed.

    SUCCESS, ///< The request completed successfully.

    CONNECT_ERROR, ///< The request had a connect error.
    CONNECT_DNS_ERROR, ///< The request couldn't lookup the DNS for the url.
    CONNECT_SSL_ERROR, ///< The request has an SSL connection error.

    TIMEOUT, ///< The request timed out.
    RESPONSE_EMPTY, ///< The request has an empty response (socket severed).

    ERROR, ///< The request had an error and failed.
    ERROR_FAILED_TO_START, ///< The request had an error and failed to start.
    DOWNLOAD_ERROR ///< The request had an error in the CURL write callback.
};

/**
 * @param request_status Convert to string.
 * @return String representation of the request status.
 */
auto to_string(
    RequestStatus request_status) -> const std::string&;

} // lift
