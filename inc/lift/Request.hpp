#pragma once

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

    Request(
        std::string url,
        std::optional<std::chrono::milliseconds> timeout = std::nullopt,
        OnCompleteHandlerType on_complete_handler = nullptr);

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

    auto Timeout() const -> const std::optional<std::chrono::milliseconds>&;
    auto Timeout(
        std::optional<std::chrono::milliseconds> timeout) -> void;

    /**
     * @return The URL of the HTTP request.
     */
    auto Url() const -> const std::string&;
    /**
     * @param url The URL of the HTTP request.
     */
    auto Url(
        std::string url) -> void;

    auto Method() const -> http::Method;
    auto Method(
        http::Method method) -> void;

    auto Version() const -> http::Version;
    auto Version(
        http::Version version) -> void;

    auto FollowRedirects() const -> bool;
    auto MaxRedirects() const -> int64_t;
    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @param max_redirects The maximum number of redirects to follow, -1 is infinite, 0 is none.
     */
    auto FollowRedirects(
        bool follow_redirects,
        int64_t max_redirects = -1) -> void;

    auto VerifySslPeer() const -> bool;
    auto VerifySslPeer(
        bool verify_ssl_peer) -> void;

    auto VerifySslHost() const -> bool;
    auto VerifySslHost(
        bool verify_ssl_host) -> void;

    auto AcceptEncodings() const -> const std::optional<std::vector<std::string>>&;
    /**
     * IMPORTANT: Using this is mutually exclusive with adding your own Accept-Encoding header.
     * 
     * @param encodings A list of accept encodings to send in the request.
     */
    auto AcceptEncoding(
        std::optional<std::vector<std::string>> encodings) -> void;

    auto ResolveHosts() const -> const std::vector<lift::ResolveHost>&;
    auto ResolveHost(
        lift::ResolveHost resolve_host) -> void;

    /**
     * Specifically removes the header from the request.  There are a few
     * default headers that are always added in certain scenarios, this method
     * allows for the user to manually remove them.
     * @param name The name of the header, e.g. 'Accept' or 'Expect'.
     */
    auto RemoveHeader(
        std::string_view name) -> void;
    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto Header(
        std::string_view name,
        std::string_view value) -> void;

    auto Headers() const -> const std::vector<HeaderView>&;

    auto RequestData() const -> const std::string&;
    /**
     * Sets the request to HTTP POST and the body of the request
     * to the provided data.
     *
     * NOTE: this is mutually exclusive with using AddMimeField or AddMimeFileField,
     * as you cannot include traditional POST data in a mime-type form submission.
     *
     * @param data The request data to send in the HTTP POST.
     * @throw std::logic_error If called after using AddMimeField.
     */
    auto RequestData(
        std::string data) -> void;

    auto MimeFields() const -> const std::vector<lift::MimeField>&;
    auto MimeField(
        lift::MimeField mime_field) -> void;

private:
    /// The on complete handler callback.
    OnCompleteHandlerType m_on_complete_handler{ nullptr };
    /// The transfer progress handler callback.
    TransferProgressHandlerType m_on_transfer_progress_handler{ nullptr };
    /// The timeout for the request, or none.
    std::optional<std::chrono::milliseconds> m_timeout{};
    /// The URL.
    std::string m_url{};
    /// The HTTP request method.
    http::Method m_method{ http::Method::GET };
    /// The HTTP version to use for this request.
    http::Version m_version{ http::Version::USE_BEST };
    /// Should this request automatically follow redirects?
    bool m_follow_redirects{ true };
    /// How many redirects should be followed? -1 infinite, 0 none, <num>.
    int64_t m_max_redirects{ -1 };
    /// Should the peer be ssl verified?
    bool m_verify_ssl_peer{ true };
    /// Should the host be ssl verified?
    bool m_verify_ssl_host{ true };
    /// Specific Accept-Encoding header fields.
    std::optional<std::vector<std::string>> m_accept_encodings{};
    /// A set of host:port to ip addresses that will be resolved before DNS.
    std::vector<lift::ResolveHost> m_resolve_hosts{};
    /// The request headers buffer to quickly append into without allocating memory.
    std::string m_request_headers{};
    /// The request headers index.  Used to generate the curl slist.
    std::vector<HeaderView> m_request_headers_idx{};
    /// The POST request body data, mutually exclusive with MimeField requests.
    bool m_request_data_set{ false };
    std::string m_request_data{};
    /// The Mime request fields, mutually exclusive with POST request body data.
    bool m_mime_fields_set{ false };
    std::vector<lift::MimeField> m_mime_fields{};

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
};

using RequestPtr = std::unique_ptr<Request>;

} // namespace lift
