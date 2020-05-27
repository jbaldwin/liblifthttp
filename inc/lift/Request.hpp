#pragma once

#include "lift/Header.hpp"
#include "lift/Http.hpp"
#include "lift/MimeField.hpp"
#include "lift/ResolveHost.hpp"
#include "lift/Response.hpp"

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lift {

class EventLoop;
class Executor;

class Request {
    friend EventLoop;
    friend Executor;

public:
    /**
     * On complete handler callback signature.
     * @param request_ptr Passes ownership of the request back to the user of liblifthttp.
     * @param response Response of the request_ptr.
     */
    using OnCompleteHandlerType = std::function<void(
        std::unique_ptr<Request> request_ptr,
        Response response)>;

    /**
     * Transfer progress handler callback signature.
     * @param download_total_bytes Total number of bytes the application should expect to download.
     * @param download_now_bytes Number of bytes that have been downloaded so far.
     * @param upload_total_bytes Total number of bytes to upload.
     * @param upload_now_bytes Number of bytes uploaded so far.
     * @return True to continue the request, false to abort the request.
     */
    using TransferProgressHandlerType = std::function<bool(
        const Request& request,
        int64_t download_total_bytes,
        int64_t download_now_bytes,
        int64_t upload_total_bytes,
        int64_t upload_now_bytes)>;

    /**
     * Creates a new request with the given url, possible timeout and possible on complete handler.
     * Note that synchronous requests do not require on complete handlers as the Perfom() function
     * will return the Result immediately to the caller.
     * 
     * @param url The url to request.
     * @param timeout An optional timeout for this request.  If not provided the request
     *                could hang/block forever if it is never responded to.
     * @param on_complete_handler For asynchronous requests provide this if you want to 
     *                            know when the request completes with the Response information.
     */
    Request(
        std::string url,
        std::optional<std::chrono::milliseconds> timeout = std::nullopt,
        OnCompleteHandlerType on_complete_handler = nullptr);

    /**
     * Creates a new request on the heap, this is a useful utility for asynchronous requests.
     *
     * Note that requests may be re-used after completing.
     *
     * @param url The url to request.
     * @param timeout An optional timeout for this request.  If not provided the request
     *                could hang/block forever if it is never responded to.
     * @param on_complete_handler For asynchronous requests provide this if you want to 
     *                            know when the request completes with the Response information.
     */
    static auto make(
        std::string url,
        std::optional<std::chrono::milliseconds> timeout = std::nullopt,
        OnCompleteHandlerType on_complete_handler = nullptr) -> std::unique_ptr<Request>
    {
        return std::make_unique<Request>(std::move(url), std::move(timeout), std::move(on_complete_handler));
    }

    Request(const Request&) = default;
    Request(Request&&) = default;
    auto operator=(const Request&) noexcept -> Request& = default;
    auto operator=(Request&&) noexcept -> Request& = default;

    /**
     * Synchronously executes this request.
     *
     * Note: If there is no timeout set on the request and the remote
     * server fails to respond this call can block forever.
     */
    auto Perform() -> Response;

    /**
     * This on complete handler event is called when a Request is executed
     * asynchronously.  This is not used for synchronous requests.
     * @param on_complete_handler When this request completes this handle is called.
     */
    auto OnCompleteHandler(
        OnCompleteHandlerType on_complete_handler) -> void;

    /**
     * Sets or unsets a transfer progress handler callback.  Called periodically to update the
     * application of the status of this requests in terms of uploaded bytes and downloaded bytes.
     * @param transfer_progress_handler If an empty optional then transfer progress callbacks are disabled,
     *                                  if set with a function then transfer progress callbacks are enabled.
     */
    auto TransferProgressHandler(
        std::optional<TransferProgressHandlerType> transfer_progress_handler) -> void;

    auto Timeout() const -> const std::optional<std::chrono::milliseconds>& { return m_timeout; }
    auto Timeout(
        std::optional<std::chrono::milliseconds> timeout) -> void { m_timeout = std::move(timeout); }

    /**
     * @return The URL of the HTTP request.
     */
    auto Url() const -> const std::string& { return m_url; }
    /**
     * @param url The URL of the HTTP request.
     */
    auto Url(
        std::string url) -> void { m_url = std::move(url); }

    auto Method() const -> http::Method { return m_method; }
    auto Method(
        http::Method method) -> void { m_method = method; }

    auto Version() const -> http::Version { return m_version; }
    auto Version(
        http::Version version) -> void { m_version = version; }

    auto FollowRedirects() const -> bool { return m_follow_redirects; }
    auto MaxRedirects() const -> int64_t { return m_max_redirects; }
    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @param max_redirects The maximum number of redirects to follow, -1 is infinite, 0 is none.
     */
    auto FollowRedirects(
        bool follow_redirects,
        int64_t max_redirects = -1) -> void;

    auto VerifySslPeer() const -> bool { return m_verify_ssl_peer; }
    auto VerifySslPeer(
        bool verify_ssl_peer) -> void { m_verify_ssl_peer = verify_ssl_peer; }

    auto VerifySslHost() const -> bool { return m_verify_ssl_host; }
    auto VerifySslHost(
        bool verify_ssl_host) -> void { m_verify_ssl_host = verify_ssl_host; }

    auto AcceptEncodings() const -> const std::optional<std::vector<std::string>>& { return m_accept_encodings; }
    /**
     * IMPORTANT: Using this is mutually exclusive with adding your own Accept-Encoding header.
     * @param encodings A list of accept encodings to send in the request.
     */
    auto AcceptEncoding(
        std::optional<std::vector<std::string>> encodings) -> void { m_accept_encodings = std::move(encodings); }

    /**
     * Sets the accept encoding header to all supported encodings that this platform was built with.
     */
    auto AcceptEncodingAllAvailable() -> void { m_accept_encodings = std::vector<std::string> {}; }

    auto ResolveHosts() const -> const std::vector<lift::ResolveHost>& { return m_resolve_hosts; }
    auto ResolveHost(
        lift::ResolveHost resolve_host) -> void { m_resolve_hosts.emplace_back(std::move(resolve_host)); }
    auto ClearResolveHosts() -> void { m_resolve_hosts.clear(); }

    /**
     * Specifically removes the header from the request.  There are a few
     * default headers that are always added in certain scenarios, this method
     * allows for the user to manually remove them.
     * @param name The name of the header, e.g. 'Accept' or 'Expect'.
     */
    auto RemoveHeader(
        std::string_view name) -> void { Header(name, std::string_view {}); }
    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto Header(
        std::string_view name,
        std::string_view value) -> void;

    /**
     * @return The current list of headers added to this request.  Note that if more headers are added
     *         the Header classes Name() and Value() string_views might become invalidated.
     */
    auto Headers() const -> const std::vector<lift::Header>& { return m_request_headers; }

    /**
     * Clears the current set of headers for this request.
     */
    auto ClearHeaders() -> void { m_request_headers.clear(); }

    auto Data() const -> const std::string& { return m_request_data; }
    /**
     * Sets the request to HTTP POST and the body of the request
     * to the provided data.  Override the method after this call to PUT if desired.
     *
     * NOTE: this is mutually exclusive with using MimeField
     * as you cannot include traditional POST data in a mime-type form submission.
     *
     * @param data The request data to send in the HTTP POST.
     * @throw std::logic_error If called after using AddMimeField.
     */
    auto Data(
        std::string data) -> void;

    auto MimeFields() const -> const std::vector<lift::MimeField>& { return m_mime_fields; }
    auto MimeField(
        lift::MimeField mime_field) -> void;

private:
    /// The on complete handler callback.
    OnCompleteHandlerType m_on_complete_handler { nullptr };
    /// The transfer progress handler callback.
    TransferProgressHandlerType m_on_transfer_progress_handler { nullptr };
    /// The timeout for the request, or none.
    std::optional<std::chrono::milliseconds> m_timeout {};
    /// The timesup for the request, or none.
    std::optional<std::chrono::milliseconds> m_timesup {};
    /// The URL.
    std::string m_url {};
    /// The HTTP request method.
    http::Method m_method { http::Method::GET };
    /// The HTTP version to use for this request.
    http::Version m_version { http::Version::USE_BEST };
    /// Should this request automatically follow redirects?
    bool m_follow_redirects { true };
    /// How many redirects should be followed? -1 infinite, 0 none, <num>.
    int64_t m_max_redirects { -1 };
    /// Should the peer be ssl verified?
    bool m_verify_ssl_peer { true };
    /// Should the host be ssl verified?
    bool m_verify_ssl_host { true };
    /// Specific Accept-Encoding header fields.
    std::optional<std::vector<std::string>> m_accept_encodings {};
    /// A set of host:port to ip addresses that will be resolved before DNS.
    std::vector<lift::ResolveHost> m_resolve_hosts {};
    /// The request headers preformatted into the curl "Header: value\0" format.
    std::vector<lift::Header> m_request_headers {};
    /// The POST request body data, mutually exclusive with MimeField requests.
    bool m_request_data_set { false };
    std::string m_request_data {};
    /// The Mime request fields, mutually exclusive with POST request body data.
    bool m_mime_fields_set { false };
    std::vector<lift::MimeField> m_mime_fields {};

    // libcurl will call this function if the user has requested transfer progress information.
    friend auto curl_xfer_info(
        void* clientp,
        curl_off_t download_total_bytes,
        curl_off_t download_now_bytes,
        curl_off_t upload_total_bytes,
        curl_off_t upload_now_bytes) -> int;
};

using RequestPtr = std::unique_ptr<Request>;

} // namespace lift
