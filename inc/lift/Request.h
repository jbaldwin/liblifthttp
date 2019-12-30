#pragma once

#include "lift/Header.h"
#include "lift/Http.h"
#include "lift/RequestStatus.h"
#include "lift/ResolveHost.h"

#include <curl/curl.h>
#include <uv.h>

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace lift {
class RequestHandle;
class RequestPool;

class Request {
    friend class EventLoop;
    friend class RequestPool;

public:
    ~Request();

    /**
     * Do not move or copy these objects anywhere, they should always be wrapped in
     * a unique_ptr for cheapness of moving their internals as well as maintaining
     * the life time of these objects in the pool
     * @{
     */
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) = delete;
    auto operator=(Request &&) -> Request& = delete;
    /** @} */

    /**
     * @param on_complete_handler When this request completes this handle is called.
     */
    auto SetOnCompleteHandler(
        std::function<void(RequestHandle)> on_complete_handler) -> void;

    /**
     * @param url The URL of the HTTP request.
     * @return True if the url was set.
     */
    auto SetUrl(
        const std::string& url) -> bool;

    /**
     * @return The currently set URL for this HTTP request.
     */
    [[nodiscard]] auto GetUrl() const -> std::string_view;

    /**
     * @param http_method Sets the HTTP method for this request.
     */
    auto SetMethod(
        http::Method http_method) -> void;

    /**
     * @param http_version Sets the HTTP version for this request.
     */
    auto SetVersion(
        http::Version http_version) -> void;

    /**
     * Sets the timeout for this HTTP request.  This should be set before Perform() is called
     * or if this is an AsyncRequest before adding to an EventLoop.
     * @param timeout The timeout for the request.
     * @return True if the timeout was set.
     */
    auto SetTimeout(
        std::chrono::milliseconds timeout) -> bool;

    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @param max_redirects The maximum number of redirects to follow, -1 is infinite, 0 is none.
     * @return True if the follow redirects was set.
     */
    auto SetFollowRedirects(
        bool follow_redirects,
        int64_t max_redirects = -1) -> bool;

    /**
     * @param verify_ssl_peer Should the peer be ssl verified?
     */
    auto SetVerifySslPeer(
        bool verify_ssl_peer) -> void;

    /**
     * @param verify_ssl_host Should the host be ssl verified?
     */
    auto SetVerifySslHost(
        bool verify_ssl_host) -> void;

    /**
     * @param resolve_host Adds a host to ip address dns resolver.
     */
    auto AddResolveHost(
        ResolveHost resolve_host) -> void;

    /**
     * Specifically removes the header from the request.
     * @param name The name of the header, e.g. 'Accept'.
     */
    auto RemoveHeader(
        std::string_view name) -> void;

    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto AddHeader(
        std::string_view name,
        std::string_view value) -> void;

    /**
     * @return The list of headers applied to this request.
     */
    [[nodiscard]] auto GetRequestHeaders() const -> const std::vector<Header>&;

    /**
     * Sets the request to HTTP POST and the body of the request
     * to the provided data.
     *
     * NOTE: this is mutually exclusive with using AddMimeField or AddMimeFileField,
     * as you cannot include traditional POST data in a mime-type form submission.
     *
     * @param data The request data to send in the HTTP POST.
     * 
     * @throws std::logic_error If called after using AddMimeField or AddMimeFileField
     */
    auto SetRequestData(
        std::string data) -> void;

    /**
     * @return The request data.  If never set an empty string is returned.
     */
    [[nodiscard]] auto GetRequestData() const -> const std::string&;

    /**
     * Adds an additional mime field to the request. This is only valid for POST
     * requests, and for submitting HTML form-like data
     *
     * NOTE: this is mutually exclusive with using SetRequestData, as you cannot
     * include traditional POST data in a mime-type form submission.
     *
     * NOTE: Fields are specifically const std::string& because the underlying curl library
     * only takes a char* that is required to be null terminated.
     *
     * @param field_name The name of the form field.
     * @param field_value The value for the form field.
     * 
     * @throws std::logic_error If called after using SetRequestData.
     */
    auto AddMimeField(
        const std::string& field_name,
        const std::string& field_value) -> void;

    /**
     * Adds an additional mime field (as a file) to the request. This is only valid
     * for POST requests, and for submitting HTML form-like data
     *
     * NOTE: this is mutually exclusive with using SetRequestData, as you cannot
     * include traditional POST data in a mime-type form submission.
     *
     * @param field_name The name of the form field, this will be the filename as received
     * by the other side.
     * @param field_filepath The path value for the form field, a file path of the file to treat
     * as a form file upload. This file must exist and be readable when this Request is
     * actually performed (e.g. the file data is streamed on demand, and isn't loaded
     * when this function is called). This path is only used on the request-side to read
     * the data, the field_name .
     * 
     * @throws std::logic_error If called after using SetRequestData.
     * @throws std::runtime_error If the file from `field_filepath` doesn't exist.
     */
    auto AddMimeField(
        const std::string& field_name,
        const std::filesystem::path& field_filepath) -> void;

    /**
     * Transfer progress handler callback signature.
     * @param download_total_bytes Total number of bytes the application should expect to download.
     * @param download_now_bytes Number of bytes that have been downloaded so far.
     * @param upload_total_bytes Total number of bytes to upload.
     * @param upload_now_bytes Number of bytes uploaded so far.
     * @return True to continue the request, false to abort the request.
     */
    using TransferProgressHandler = std::function<bool(
        const Request& request,
        int64_t download_total_bytes,
        int64_t download_now_bytes,
        int64_t upload_total_bytes,
        int64_t upload_now_bytes)>;

    /**
     * Sets or unsets a transfer progress handler callback.  Called periodically to update the
     * application of the status of this requests in terms of uploaded bytes and downloaded bytes.
     * @param transfer_progress_handler If an empty optional then transfer progress callbacks are disabled,
     *                                  if set with a function then transfer progress callbacks are enabled.
     */
    auto SetTransferProgressHandler(
        std::optional<TransferProgressHandler> transfer_progress_handler) -> void;

    /**
     * Performs the HTTP request synchronously.  This call will block the calling thread.
     * @return True if the request was successful.
     */
    auto Perform() -> bool;

    /**
     * @return The HTTP response status code.
     */
    [[nodiscard]] auto GetResponseStatusCode() const -> http::StatusCode;

    /**
     * @return The HTTP response headers.
     */
    [[nodiscard]] auto GetResponseHeaders() const -> const std::vector<Header>&;

    /**
     * @return The HTTP download payload.
     */
    [[nodiscard]] auto GetResponseData() const -> const std::string&;

    /**
     * @return The total HTTP request time in milliseconds.
     */
    [[nodiscard]] auto GetTotalTime() const -> std::chrono::milliseconds;

    /**
     * The completion status is how the request ended up in the event loop.
     * It might have completed successfully, or timed out, or had an SSL error, etc.
     *
     * This is not the HTTP status code returned by the remote server.
     *
     * @return Gets the request completion status.
     */
    [[nodiscard]] auto GetCompletionStatus() const -> RequestStatus;

    /**
     * @return The number of connections made to make this request
     */
    [[nodiscard]] auto GetNumConnects() const -> uint64_t;

    /**
     * @return The number of redirects made during this request.
     */
    [[nodiscard]] auto GetNumRedirects() const -> uint64_t;

    /**
     * Resets the request to be re-used.  This will clear everything on the request.
     */
    auto Reset() -> void;

private:
    /**
     * Private constructor -- only the RequestPool can create new Requests.
     * @param request_pool The request pool that generated this handle.
     * @param url          The url for the request.
     * @param timeout      The timeout for the request in milliseconds.
     * @param on_complete_handler Function to be called when the Request finishes.
     */
    Request(
        RequestPool& request_pool,
        const std::string& url,
        std::chrono::milliseconds timeout,
        std::function<void(RequestHandle)> on_complete_handler = nullptr);

    auto init() -> void;

    /// The onComplete() handler for asynchronous requests.
    std::function<void(RequestHandle)> m_on_complete_handler{ nullptr };

    /// The transfer progress callback, this is optionally provided by the user.
    TransferProgressHandler m_on_transfer_progress_handler{ nullptr };

    /// The request pool this request was produced from.
    RequestPool& m_request_pool;

    /// The cURL handle for this request.
    CURL* m_curl_handle{ curl_easy_init() };

    /// A view into the curl url.
    std::string_view m_url{};
    /// The request headers.
    std::string m_request_headers{};
    /// The request headers index.  Used to generate the curl slist.
    std::vector<Header> m_request_headers_idx{};
    /// The curl request headers.
    curl_slist* m_curl_request_headers{ nullptr };
    /// Have the headers been committed into cURL?
    bool m_headers_committed{ false };
    /// The request data if any. Mutually exclusive with m_mime_handle.
    std::string m_request_data{};
    /// The mime handle, if any (only created when needed). Mutually exclusive with m_request_data.
    curl_mime* m_mime_handle{ nullptr };

    /// The status of this HTTP request.
    RequestStatus m_status_code{ RequestStatus::BUILDING };
    /// The response headers.
    std::string m_response_headers{};
    /// Views into each header.
    std::vector<Header> m_response_headers_idx{};
    /// The response data if any.
    std::string m_response_data{};

    /// A set of host:port to ip addresses that will be resolved before DNS
    std::vector<ResolveHost> m_resolve_hosts;
    /// The curl resolve hosts list.
    curl_slist* m_curl_resolve_hosts{ nullptr };
    /// Have the resolve hosts been updated recently?
    bool m_resolve_hosts_committed{ false };

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
        CURLcode curl_code) -> void;

    /// libcurl will call this function when a header is received for the HTTP request.
    friend auto curl_write_header(
        char* buffer,
        size_t size,
        size_t nitems,
        void* user_ptr) -> size_t;

    /// libcurl will call this function when data is received for the HTTP request.
    friend auto curl_write_data(
        void* buffer,
        size_t size,
        size_t nitems,
        void* user_ptr) -> size_t;

    /// libcurl will call this function if the user has requested transfer progress information.
    friend auto curl_xfer_info(
        void* clientp,
        curl_off_t download_total_bytes,
        curl_off_t download_now_bytes,
        curl_off_t upload_total_bytes,
        curl_off_t upload_now_bytes) -> int;

    /// libuv will call this function when the AddRequest() function is called.
    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;
};

} // namespace lift
