#pragma once

#include <string>

namespace lift {

/**
 * This is the status of a request/response pair locally.  It does not
 * have anything in common with the http::StatusCode.  This will tell you
 * if the request/response pair failed to connect, had a dns error, had an
 * ssl error, timed out, or any other various errors, or if it successfully
 * completed!  Always check this value on the lift::Response before using
 * any other data.
 */
enum class LiftStatus {
    /// The request is under construction.
    BUILDING,
    /// The request is being executed.
    EXECUTING,

    /// The request completed successfully.  This is the one you want.
    SUCCESS,

    /// The request had a connect error.
    CONNECT_ERROR,
    /// The request couldn't lookup the DNS for the url.
    CONNECT_DNS_ERROR,
    /// The request has an SSL connection error.
    CONNECT_SSL_ERROR,

    /// The request timed out.
    TIMEOUT,
    /// The request has an empty response (socket severed).
    RESPONSE_EMPTY,

    /// The request had an error and failed.
    ERROR,
    /// The request had an error and failed to start, did the event loop shutdown?
    ERROR_FAILED_TO_START,
    /// The request had an error when attempting to read data off the socket.
    DOWNLOAD_ERROR
};

/**
 * @param request_status Convert to string.
 * @return String representation of the request status.
 */
auto to_string(
    LiftStatus status) -> const std::string&;

} // lift
