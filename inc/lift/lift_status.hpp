#pragma once

#include <string>
#include <cstdint>

namespace lift
{
/**
 * This is the status of a request/response pair locally.  It does not
 * have anything in common with the http::status_code.  This will tell you
 * if the request/response pair failed to connect, had a dns error, had an
 * ssl error, timed out, or any other various errors, or if it successfully
 * completed!  Always check this value on the lift::response before using
 * any other data.
 */
enum class lift_status : uint8_t
{
    /// The request is under construction.
    building,
    /// The request is being executed.
    executing,

    /// The request completed successfully.  This is the one you want.
    success,

    /// The request had a connect error.
    connect_error,
    /// The request couldn't lookup the DNS for the url.
    connect_dns_error,
    /// The request has an SSL connection error.
    connect_ssl_error,

    /// The request timed out.
    timeout,
    /// The request has an empty response (socket severed).
    response_empty,

    /// The request had an error and failed.
    error,
    /// The request had an error and failed to start, did the event loop shutdown?
    error_failed_to_start,
    /// The request had an error when attempting to read data off the socket.
    download_error
};

/**
 * @param status Convert the lift status to human readable string.
 * @return String representation of the status.
 */
auto to_string(lift_status status) -> const std::string&;

} // namespace lift
