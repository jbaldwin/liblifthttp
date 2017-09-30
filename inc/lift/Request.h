#pragma once

#include "lift/Types.h"
#include "lift/RequestStatus.h"
#include "lift/Header.h"

#include <curl/curl.h>
#include <uv.h>

#include <string>
#include <vector>

namespace lift
{

class Request
{
public:
    /**
     * Creates an empty request.
     */
    Request();

    /**
     * Creates a request with a url and timeout.
     * @param url The request url.
     * @param timeout_ms The timeout for this request in milliseconds.  Defaults to 0,
     *                   where 0 is an infinite timeout.
     */
    Request(
        const std::string& url,
        uint64_t timeout_ms = 0
    );

    virtual ~Request();

    /**
     * @param copy No copying allowed.
     */
    Request(const Request& copy) = delete;

    /**
     * @param move Moving is allowed.
     */
    Request(Request&& move) = default;

    /**
     * @param copy_assign No copy assignment allowed.
     */
    auto operator=(const Request& copy_assign) = delete;

    /**
     * @param move_assign Move assignemnt allowed.
     */
    auto operator=(Request&& move_assign) -> Request& = default;

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

protected:
    StringView m_url;              ///< A view into the curl url.
    CURL* m_curl_handle;            ///< The curl easy handle for this request.
    RequestStatus m_status_code;    ///< The status of this HTTP request.
    std::string m_response_headers; ///< The response headers.
    std::vector<Header> m_response_headers_idx; ///< Views into each header.
    std::string m_response_data;    ///< The response data if any.

    /**
     * Converts a CURLcode into a RequestStatus.
     * @param curl_code The CURLcode to convert.
     * @return RequestStatus
     */
    static auto curl_code2request_status(
        CURLcode curl_code
    ) -> RequestStatus;

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
};

} // lift
