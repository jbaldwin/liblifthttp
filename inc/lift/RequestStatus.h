#pragma once

namespace lift
{

enum class RequestStatus
{
    SUCCESS,            ///< The request completed succesfully.

    CONNECT_ERROR,      ///< The request had a connect error.
    CONNECT_DNS_ERROR,  ///< The request couldn't lookup the DNS for the url.
    CONNECT_SSL_ERROR,  ///< The request has an SSL connection error.

    TIMEOUT,            ///< The request timed out.
    RESPONSE_EMPTY,     ///< The request has an empty response (socket severed).

    ERROR               ///< The request had an error and failed.
};

} // lift
