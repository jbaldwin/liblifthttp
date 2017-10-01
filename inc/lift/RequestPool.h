#pragma once

#include "lift/Request.h"

#include <deque>
#include <memory>

namespace lift
{

class CurlPool;

class RequestPool
{
public:
    RequestPool();
    ~RequestPool();

    RequestPool(const RequestPool&) = delete;                       ///< No copying
    RequestPool(RequestPool&&) = default;                           ///< Can move
    auto operator = (const RequestPool&) -> RequestPool& = delete;  ///< No copy assign
    auto operator = (RequestPool&&) -> RequestPool& = default;      ///< Can move assign

    /**
     * Produces a new Request.
     *
     * This function is thread safe.
     *
     * @param url The url of the Request.
     * @param timeout_ms The timeout of the request in milliseconds.
     * @return A Request object setup for the URL + Timeout.
     */
    auto Produce(
        const std::string& url,
        uint64_t timeout_ms = 0
    ) -> std::unique_ptr<Request>;

    /**
     * Returns a Request object to the pool to be re-used.
     *
     * This function is thread safe.
     *
     * @param request The request to return to the pool to be re-used.
     */
    auto Return(
        std::unique_ptr<Request> request
    ) -> void;

private:
    std::mutex m_lock;                                  ///< Used for thread safe calls.
    std::deque<std::unique_ptr<Request>> m_requests;    ///< Pool of un-used Request handles.
    std::unique_ptr<CurlPool> m_curl_pool;              ///< Pool of CURL* handles.
};

} // lift
