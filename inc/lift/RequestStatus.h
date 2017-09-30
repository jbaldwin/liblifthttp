#pragma once

namespace lift
{

enum class RequestStatus
{
    SUCCESS,

    CONNECT_ERROR,
    CONNECT_DNS_ERROR,
    CONNECT_SSL_ERROR,

    TIMEOUT,
    RESPONSE_EMPTY,

    ERROR
};

} // lift
