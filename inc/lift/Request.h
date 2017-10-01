#pragma once

#include "lift/Types.h"
#include "lift/RequestStatus.h"
#include "lift/Header.h"

#include <curl/curl.h>
#include <uv.h>

#include <string>
#include <vector>
#include <list>

namespace lift
{

class CurlPool;

class Request
{
    friend class EventLoop;
    friend class RequestPool;

public:
    ~Request();

    Request(const Request&) = delete;                  ///< No copying
    Request(Request&&) = default;                      ///< Can move
    auto operator = (const Request&) = delete;         ///< No copy assign
    auto operator = (Request&&) -> Request& = default; ///< Can move assign

    /**
     * @param url The URL of the HTTP request.
     * @return True if the url was set.
     */
    auto SetUrl(
        const std::string& url
    ) -> bool;

    /**
     * @return The currently set URL for this HTTP request.
     */
    auto GetUrl() const -> StringView;

    /**
     * Sets the timeout for this HTTP request.  This should be set before Perform() is called
     * or if this is an AsyncRequest before adding to an EventLoop.
     * @param timeout_ms The timeout in milliseconds.
     * @return True if the timeout was set.
     */
    auto SetTimeoutMilliseconds(
        uint64_t timeout_ms
    ) -> bool;

    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @return True if the follow redirects was set.
     */
    auto SetFollowRedirects(
        bool follow_redirects
    ) -> bool;

    /**
     * Performs the HTTP request synchronously.  This call will block the calling thread.
     * @return True if the request was successful.
     */
    auto Perform() -> bool;

    /**
     * @return The HTTP response headers.
     */
    auto GetResponseHeaders() const -> const std::vector<Header>&;

    /**
     * @return The HTTP download payload.
     */
    auto GetResponseData() const -> const std::string&;

    /**
     * @return The total HTTP request time in milliseconds.
     */
    auto GetTotalTimeMilliseconds() const -> uint64_t;

    /**
     * @return True if the HTTP request has had an error.
     */
    auto HasError() const -> bool;

    /**
     * @return Gets the request error.
     */
    auto GetStatus() const -> RequestStatus;

    /**
     * Resets the request to be re-used.  This will clear everything on the request.
     */
    auto Reset() -> void;

private:
    /**
     * Private constructor -- only the RequestPool can create new Requests.
     * @param url         The url for the request.
     * @param timeout_ms  The timeout for the request in milliseconds.
     * @param curl_handle The CURL* handle for this Request.
     * @param curl_pool   The CurlPool to return the CURL* handle when this request destructs.
     */
    explicit Request(
        const std::string& url,
        uint64_t timeout_ms,
        CURL* curl_handle,
        CurlPool& curl_pool
    );

    StringView m_url;                           ///< A view into the curl url.
    CURL* m_curl_handle;                        ///< The curl handle for this request.
    CurlPool& m_curl_pool;                      ///< The curl handle pool.
    RequestStatus m_status_code;                ///< The status of this HTTP request.
    // TODO: merge into a single large buffer
    std::string m_response_headers;             ///< The response headers.
    std::vector<Header> m_response_headers_idx; ///< Views into each header.
    std::string m_response_data;                ///< The response data if any.

    /**
     * If this Request is a part of an asynchronous event loop this is the position
     * in the event loops internal list.
     */
    std::list<std::unique_ptr<Request>>::iterator m_active_requests_position;

    /**
     * Converts a CURLcode into a RequestStatus.
     * @param curl_code The CURLcode to convert.
     */
    auto setRequestStatus(
        CURLcode curl_code
    ) -> void;

    friend auto curl_write_header(
        char* buffer,
        size_t size,
        size_t nitems,
        void* user_ptr
    ) -> size_t; ///< libcurl will call this function when a header is received for the HTTP request.

    friend auto curl_write_data(
        void* buffer,
        size_t size,
        size_t nitems,
        void* user_ptr
    ) -> size_t; ///< libcurl will call this function when data is received for the HTTP request.

    friend auto requests_accept_async(
        uv_async_t* async,
        int status
    ) -> void; ///< libuv will call this function when the AddRequest() function is called.
};

} // lift
