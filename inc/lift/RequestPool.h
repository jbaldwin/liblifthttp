#pragma once

#include "lift/Request.h"
#include "lift/RequestHandle.h"

#include <deque>
#include <memory>
#include <chrono>
#include <mutex>
#include <functional>

namespace lift
{

class CurlPool;

class RequestPool
{
    friend class Request;

public:
    RequestPool();
    ~RequestPool();

    RequestPool(const RequestPool&) = delete;                       ///< No copying
    RequestPool(RequestPool&&) = default;                           ///< Can move
    auto operator = (const RequestPool&) -> RequestPool& = delete;  ///< No copy assign
    auto operator = (RequestPool&&) -> RequestPool& = default;      ///< Can move assign

    /**
     * Produces a new Request with infinite timeout.
     *
     * This produce method is best used for synchronous requests.
     *
     * @param url The url of the Request.
     * @return A request object setup for the URL.
     */
    auto Produce(
        const std::string& url
    ) -> Request;

    /**
     * Produces a new Request with the specified timeout.
     *
     * This produce method is best used for synchronous requests.
     *
     * @param url The url of the Request.
     * @param timeout The timeout of the request.
     * @return A Request object setup for the URL + Timeout.
     */
    auto Produce(
        const std::string& url,
        std::chrono::milliseconds timeout
    ) -> Request;

    /**
     * Produces a new Request.  This function is thread safe.
     *
     * This produce method is best used for asynchronous requests.
     *
     * @param url The url of the Request.
     * @param on_complete_handler The on completion callback handler for this request.
     * @param timeout The timeout of the request.
     * @return A Request object setup for the URL + Timeout.
     */
    auto Produce(
        const std::string& url,
        std::function<void(Request)> on_complete_handler,
        std::chrono::milliseconds timeout
    ) -> Request;

private:
    std::mutex m_lock;                                      ///< Used for thread safe calls.
    std::deque<std::unique_ptr<RequestHandle>> m_requests;  ///< Pool of un-used Request handles.
    std::unique_ptr<CurlPool> m_curl_pool;                  ///< Pool of CURL* handles.

    /**
     * Returns a Request object to the pool to be re-used.
     *
     * This function is thread safe.
     *
     * @param request The request to return to the pool to be re-used.
     */
    auto returnRequest(
        std::unique_ptr<RequestHandle> request
    ) -> void;
};

} // lift
