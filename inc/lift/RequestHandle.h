#pragma once

#include "lift/RequestStatus.h"
#include "lift/Header.h"
#include "lift/Http.h"

#include <curl/curl.h>
#include <uv.h>

#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace lift
{

class Request;
class RequestPool;
class CurlPool;

typedef void(*OnCompleteHandler)(Request);

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
     * @param on_complete_handler When this request completes this handle is called.
     */
    auto SetOnCompleteHandler(
        OnCompleteHandler on_complete_handler
    ) -> void;

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
    auto GetUrl() const -> std::string_view;

    /**
     * @param http_method Sets the HTTP method for this request.
     */
    auto SetMethod(
        http::Method http_method
    ) -> void;

    /**
     * @param http_version Sets the HTTP version for this request.
     */
    auto SetVersion(
        http::Version  http_version
    ) -> void;

    /**
     * Sets the timeout for this HTTP request.  This should be set before Perform() is called
     * or if this is an AsyncRequest before adding to an EventLoop.
     * @param timeout The timeout for the request.
     * @return True if the timeout was set.
     */
    auto SetTimeout(
        std::chrono::milliseconds timeout
    ) -> bool;

    /**
     * Sets the maximum number of bytes of data to write.
     *
     * To download a full file, set max_download_bytes to -1.
     *
     * The number of bytes downloaded may be greater than the set amount,
     * but the number of bytes written for the response's data will not
     * exceed this amount.
     * @param max_download_bytes     The maximum number of bytes to be written for this request.
     */
    auto SetMaxDownloadBytes(
        ssize_t max_download_bytes
    ) -> void;

    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @param max_redirects The maximum number of redirects to follow, -1 is infinite, 0 is none.
     * @return True if the follow redirects was set.
     */
    auto SetFollowRedirects(
        bool follow_redirects,
        int64_t max_redirects = -1
    ) -> bool;

    /**
     * Adds a request header with an empty value.
     * @param name The name of the header, e.g. 'Accept'.
     */
    auto AddHeader(
        std::string_view name
    ) -> void;

    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto AddHeader(
        std::string_view name,
        std::string_view value
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
     * @return The HTTP response code.
     */
    auto GetResponseCode() const -> int64_t;

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
     * The completion status is how the request ended up in the event loop.
     * It might have completed successfully, or timed out, or had an SSL error, etc.
     *
     * This is not the HTTP status code returned by the remote server.
     *
     * @return Gets the request completion status.
     */
    auto GetCompletionStatus() const -> RequestStatus;

    /**
     * Resets the request to be re-used.  This will clear everything on the request.
     */
    auto Reset() -> void;

    /**
     * Sets a user provided data pointer.  The Request object in no way
     * owns this data and is simply pass through to OnComplete().
     * @param user_data The data pointer
     */
    auto SetUserData(
        void* user_data
    ) -> void;

    /**
     * @return Gets the user provided data if any.
     */
    auto GetUserData() -> void*;

private:
    /**
     * Private constructor -- only the RequestPool can create new Requests.
     * @param url          The url for the request.
     * @param timeout      The timeout for the request in milliseconds.
     * @param request_pool The request pool that generated this handle.
     * @param curl_handle  The CURL* handle for this Request.
     * @param curl_pool    The CurlPool to return the CURL* handle when this request destructs.
     * @param on_complete_handler   Function to be called when the CURL request finishes.
     * @param max_download_bytes    The maximum number of bytes to download, if -1, will download entire file.
     */
    explicit RequestHandle(
        const std::string& url,
        std::chrono::milliseconds timeout,
        RequestPool& request_pool,
        CURL* curl_handle,
        CurlPool& curl_pool,
        OnCompleteHandler on_complete_handler = nullptr,
        ssize_t max_download_bytes = -1
    );

    auto init() -> void;

    OnCompleteHandler m_on_complete_handler;    ///< The onComplete() handler.

    RequestPool& m_request_pool;                ///< The request pool this request was produced from.

    CURL* m_curl_handle;                        ///< The curl handle for this request.
    CurlPool& m_curl_pool;                      ///< The curl handle pool.

    std::string_view m_url;                     ///< A view into the curl url.
    std::string m_request_headers;              ///< The request headers.
    std::vector<Header> m_request_headers_idx;  ///< The request headers index.
    curl_slist* m_curl_request_headers;         ///< The curl request headers.
    bool m_headers_committed;                   ///< Have the headers been committed into CURL?
    std::string m_request_data;                 ///< The request data if any.

    RequestStatus m_status_code;                ///< The status of this HTTP request.
    // TODO: merge into a single large buffer
    std::string m_response_headers;             ///< The response headers.
    std::vector<Header> m_response_headers_idx; ///< Views into each header.
    std::string m_response_data;                ///< The response data if any.

    void* m_user_data;                          ///< The user data.

    ssize_t m_max_download_bytes;               ///< Maximum number of bytes to be written.
    ssize_t m_bytes_written;                    ///< Number of bytes that have been written so far.

    /**
     * Prepares the request to be performed.  This is called on a request
     * before it is sync or async executed.
     *
     * Commits the request headers if any are available.
     */
    auto prepareForPerform() -> void;

    /**
     * Clears response buffers unrelated to curl.  This is useful if you want
     * to make the exact same request multiple times.
     */
    auto clearResponseBuffers() -> void;

    /**
     * Converts a CURLcode into a RequestStatus.
     * @param curl_code The CURLcode to convert.
     */
    auto setCompletionStatus(
        CURLcode curl_code
    ) -> void;

    auto onComplete() -> void;

    /**
     * Helper function to find how many bytes are left to be downloaded for a request
     * @return ssize_t found by subtracting total number of downloaded bytes from max_download_bytes
     */
    auto getRemainingDownloadBytes() -> ssize_t;

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
        uv_async_t* async
    ) -> void; ///< libuv will call this function when the AddRequest() function is called.

};

} // lift
