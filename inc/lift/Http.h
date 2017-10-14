#pragma once

namespace lift
{
namespace http
{

enum class Method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    PATCH
};

enum class Version
{
    USE_BEST,       ///< Use the best version available.
    V1_0,           ///< Use HTTP 1.0.
    V1_1,           ///< Use HTTP 1.1.
    V2_0,           ///< Attempt HTTP 2 requests but fallback to 1.1 on failure.
    V2_0_TLS,       ///< Attempt HTTP 2 over TLS (HTTPS) but fallback to 1.1 on failure.
    V2_0_ONLY       ///< Use HTTP 2.0 non-TLS with no fallback to 1.1.
};

} // http
} // lift
