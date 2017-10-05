#pragma once

#include "lift/Types.h"
#include "lift/RequestStatus.h"
#include "lift/Header.h"
#include "lift/Method.h"

#include <curl/curl.h>
#include <uv.h>

#include <string>
#include <vector>
#include <list>
#include <chrono>

namespace lift
{

class Request;
class CurlPool;

class RequestHandle
{
    friend class EventLoop;
    friend class RequestPool;

public:
    ~RequestHandle();

    RequestHandle(const RequestHandle&) = delete;                   ///< No copying
    RequestHandle(RequestHandle&&) = default;                       ///< Can move
    auto operator = (const RequestHandle&) = delete;                ///< No copy assign
    auto operator = (RequestHandle&&) -> RequestHandle& = default;  ///< Can move assign

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
     * @param http_method Sets the HTTP method for this request.
     */
    auto SetMethod(
        Method http_method
    ) -> void;

    /**
     * Sets the timeout for this HTTP request.  This should be set before Perform() is called
     * or if this is an AsyncRequest before adding to an EventLoop.
     * @param timeout The timeout for the request.
     * @return True if the timeout was set.
     */
    template<typename Rep, typename Period>
    auto SetTimeout(
        std::chrono::duration<Rep, Period> timeout
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
     * Adds a request header with an empty value.
     * @param name The name of the header, e.g. 'Accept'.
     */
    auto AddHeader(
        StringView name
    ) -> void;

    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto AddHeader(
        StringView name,
        StringView value
    ) -> void;

    /**
     * @return The list of headers applied to this request.
     */
    auto GetRequestHeaders() const -> const std::vector<Header>&;

    /**
     * Sets the request to HTTP POST and the body of the request
     * to the provided data.
     * @param data The request data to send in the HTTP POST.
     */
    auto SetRequestData(
        std::string data
    ) -> void;

    /**
     * @return The request data.  If never set an empty string is returned.
     */
    auto GetRequestData() const -> const std::string&;

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
     * @param timeout     The timeout for the request in milliseconds.
     * @param curl_handle The CURL* handle for this Request.
     * @param curl_pool   The CurlPool to return the CURL* handle when this request destructs.
     */
    explicit RequestHandle(
        const std::string& url,
        std::chrono::milliseconds timeout,
        CURL* curl_handle,
        CurlPool& curl_pool
    );

    auto init() -> void;

    CURL* m_curl_handle;                        ///< The curl handle for this request.
    CurlPool& m_curl_pool;                      ///< The curl handle pool.

    StringView m_url;                           ///< A view into the curl url.
    std::string m_request_headers;              ///< The request headers.
    std::vector<Header> m_request_headers_idx;  ///< The request headers index.
    curl_slist* m_curl_request_headers;         ///< The curl request headers.
    std::string m_request_data;                 ///< The request data if any.

    RequestStatus m_status_code;                ///< The status of this HTTP request.
    // TODO: merge into a single large buffer
    std::string m_response_headers;             ///< The response headers.
    std::vector<Header> m_response_headers_idx; ///< Views into each header.
    std::string m_response_data;                ///< The response data if any.

    /**
     * If this Request is a part of an asynchronous event loop this is the position
     * in the event loops internal list.
     */
    std::list<Request>::iterator m_active_requests_position;

    /**
     * Prepares the request to be performed.  This is called on a request
     * before it is sync or async executed.
     *
     * Commits the request headers if any are available.
     */
    auto prepareForPerform() -> void;

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

#include "lift/RequestHandle.tcc"
