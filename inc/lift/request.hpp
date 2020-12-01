#pragma once

#include "lift/header.hpp"
#include "lift/http.hpp"
#include "lift/mime_field.hpp"
#include "lift/resolve_host.hpp"
#include "lift/response.hpp"
#include "lift/share.hpp"

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lift
{
class client;
class executor;

enum class ssl_certificate_type
{
    pem,
    der
};

enum class proxy_type
{
    http,
    https
};

enum class http_auth_type
{
    /// Basic HTTP authentication, this is the default value.
    basic,
    /// Sets all available authentication methods and attempts to pick the most secure.
    any,
    /// Sets all available 'secure/safe' authentication methods.
    any_safe
    // TODO: Support setting individual http authentication methods.
};

struct proxy_data
{
    /// The type of HTTP proxy to connect to, HTTP or HTTPS.
    proxy_type m_type;
    /// The proxy hostname to connect to.
    std::string m_host;
    /// The proxy port to connect to.
    uint32_t m_port{80};
    /// The username for authentication with the proxy.
    std::optional<std::string> m_username;
    /// The password for authentication with the proxy.
    std::optional<std::string> m_password;
    /// The authentication type(s) to use for communication with the proxy, if not specified ANY is used.
    std::optional<std::vector<http_auth_type>> m_auth_types;
};

auto to_string(ssl_certificate_type type) -> const std::string&;

class request
{
    friend client;
    friend executor;

public:
    /**
     * On complete handler callback signature.
     * @param request_ptr Passes ownership of the request back to the user of liblifthttp.
     * @param response Response of the request_ptr.
     */
    using on_complete_handler_type = std::function<void(std::unique_ptr<request> request_ptr, response response)>;

    /**
     * Transfer progress handler callback signature.
     * @param download_total_bytes Total number of bytes the application should expect to download.
     * @param download_now_bytes Number of bytes that have been downloaded so far.
     * @param upload_total_bytes Total number of bytes to upload.
     * @param upload_now_bytes Number of bytes uploaded so far.
     * @return True to continue the request, false to abort the request.
     */
    using transfer_progress_handler_type = std::function<bool(
        const request& request,
        int64_t        download_total_bytes,
        int64_t        download_now_bytes,
        int64_t        upload_total_bytes,
        int64_t        upload_now_bytes)>;

    /**
     * Creates a new request with the given url, possible timeout and possible on complete handler.
     * Note that synchronous requests do not require on complete handlers as the Perfom() function
     * will return the Result immediately to the caller.
     *
     * @param url The url to request.
     * @param timeout An optional timeout for this request.  If not provided the request
     *                could hang/block forever if it is never responded to.
     * @param on_complete_handler For asynchronous requests provide this if you want to
     *                            know when the request completes with the response information.
     */
    explicit request(
        std::string                              url,
        std::optional<std::chrono::milliseconds> timeout             = std::nullopt,
        on_complete_handler_type                 on_complete_handler = nullptr);

    /**
     * Creates a new request on the heap, this is a useful utility for asynchronous requests.
     *
     * Note that requests may be re-used after completing.
     *
     * @param url The url to request.
     * @param timeout An optional timeout for this request.  If not provided the request
     *                could hang/block forever if it is never responded to.
     * @param on_complete_handler For asynchronous requests provide this if you want to
     *                            know when the request completes with the response information.
     */
    static auto make_unique(
        std::string                              url,
        std::optional<std::chrono::milliseconds> timeout             = std::nullopt,
        on_complete_handler_type                 on_complete_handler = nullptr) -> std::unique_ptr<request>
    {
        return std::make_unique<request>(std::move(url), std::move(timeout), std::move(on_complete_handler));
    }

    request(const request&) = default;
    request(request&&)      = default;
    auto operator=(const request&) noexcept -> request& = default;
    auto operator=(request&&) noexcept -> request& = default;

    /**
     * Synchronously executes this request.
     *
     * Note: If there is no timeout set on the request and the remote
     * server fails to respond this call can block forever.
     *
     * @param share_ptr An optional lift::share for reusing/sharing connection information.
     * @return The HTTP response.
     */
    auto perform(share_ptr share_ptr = nullptr) -> response;

    /**
     * This on complete handler event is called when a request is executed
     * asynchronously.  This is not used for synchronous requests.
     * @param on_complete_handler When this request completes this handle is called.
     */
    auto on_complete_handler(on_complete_handler_type on_complete_handler) -> void;

    /**
     * @return The current on complete handler callback.
     */
    auto on_complete_handler() const -> const on_complete_handler_type& { return m_on_complete_handler; }

    /**
     * Sets or unsets a transfer progress handler callback.  Called periodically to update the
     * application of the status of this requests in terms of uploaded bytes and downloaded bytes.
     * @param transfer_progress_handler If an empty optional then transfer progress callbacks are disabled,
     *                                  if set with a function then transfer progress callbacks are enabled.
     */
    auto transfer_progress_handler(std::optional<transfer_progress_handler_type> transfer_progress_handler) -> void;

    /**
     * @return The amount of time for the request to connect, or std::nullopt signals the default, 300s.
     */
    auto connect_timeout() const -> const std::optional<std::chrono::milliseconds>& { return m_connect_timeout; }

    /**
     * This value for synchronous requests should be less than the total timeout value for the entire request.
     * For requests run asynchronously through the event loop it can be longer for often repeated requests
     * to allow for connections to establish but then require requests on keep-alive connections to timeout
     * very quickly.
     * @param connect_timeout The amount of time for the request to connect, or std::nullopt to use the default, 300s.
     */
    auto connect_timeout(std::optional<std::chrono::milliseconds> connect_timeout) -> void
    {
        m_connect_timeout = std::move(connect_timeout);
    }

    /**
     * @return The amount of time for the request to complete, or std::nullopt if no timeout is set.
     */
    auto timeout() const -> const std::optional<std::chrono::milliseconds>& { return m_timeout; }

    /**
     * @param timeout The amount of time for the request to complete, or std::nullopt for no timeout.
     */
    auto timeout(std::optional<std::chrono::milliseconds> timeout) -> void { m_timeout = std::move(timeout); }

    /**
     * @return The URL of the HTTP request.
     */
    auto url() const -> const std::string& { return m_url; }
    /**
     * @param url The URL of the HTTP request.
     */
    auto url(std::string url) -> void { m_url = std::move(url); }

    /**
     * @return The HTTP method this request will use.
     */
    auto method() const -> http::method { return m_method; }

    /**
     * @param method The HTTP method this request should use.
     */
    auto method(http::method method) -> void { m_method = method; }

    /**
     * @return The HTTP version this request will use.
     */
    auto version() const -> http::version { return m_version; }

    /**
     * @param version The HTTP version this request should use.
     */
    auto version(http::version version) -> void { m_version = version; }

    /**
     * @return Is the HTTP request automatically following redirects?
     */
    auto follow_redirects() const -> bool { return m_follow_redirects; }

    /**
     * @return If following HTTP redirects, what is the maximum allowed to follow?
     */
    auto max_redirects() const -> int64_t { return m_max_redirects; }

    /**
     * Sets if this request should follow redirects.  By default following redirects is
     * enabled.
     * @param follow_redirects True to follow redirects, false to stop.
     * @param max_redirects The maximum number of redirects to follow, if not provided then infinite.
     */
    auto follow_redirects(bool follow_redirects, std::optional<uint64_t> max_redirects = std::nullopt) -> void;

    /**
     * @return Is the peer SSL/TLS verified?
     */
    auto verify_ssl_peer() const -> bool { return m_verify_ssl_peer; }

    /**
     * This feature defaults to enabled.
     * @param verify_ssl_peer Should the SSl/TLS peer be verified?
     */
    auto verify_ssl_peer(bool verify_ssl_peer) -> void { m_verify_ssl_peer = verify_ssl_peer; }

    /**
     * This feature defaultes to enabled.
     * @return Is the SSL/TLS host verified?
     */
    auto verify_ssl_host() const -> bool { return m_verify_ssl_host; }

    /**
     * @param verify_ssl_host Should the SSL/TLS host be verified?
     */
    auto verify_ssl_host(bool verify_ssl_host) -> void { m_verify_ssl_host = verify_ssl_host; }

    /**
     * @param verify_ssl_status Should the SSL/TLS certificate status be checked?
     */
    auto verify_ssl_status(bool verify_ssl_status) -> void { m_verify_ssl_status = verify_ssl_status; }

    /**
     * @return Is the SSL/TLS certificate status be checked?
     */
    auto verify_ssl_status() const -> bool { return m_verify_ssl_status; }

    /**
     * @param cert_file The SSL/TLS certificate file to use.
     */
    auto ssl_cert(std::filesystem::path cert_file) -> void { m_cert_file = std::move(cert_file); }

    /**
     * @return The SSL/TLS certificate file being used.
     */
    auto ssl_cert() const -> const std::optional<std::filesystem::path>& { return m_cert_file; }

    /**
     * @param type The SSL/TLS certificate type.
     */
    auto ssl_cert_type(ssl_certificate_type type) -> void { m_ssl_cert_type = type; }

    /**
     * @return The SSL/TSL certificate type being used.
     */
    auto ssl_cert_type() const -> const std::optional<ssl_certificate_type>& { return m_ssl_cert_type; }

    /**
     * @param key_file The SSL/TLS key file to use.
     */
    auto ssl_key(std::filesystem::path key_file) -> void { m_ssl_key_file = std::move(key_file); }

    /**
     * @return The SSL/TLS key file being used.
     */
    auto ssl_key() const -> const std::optional<std::filesystem::path>& { return m_ssl_key_file; }

    /**
     * @param password The pass phrase for the private key.
     */
    auto key_password(std::string password) -> void { m_password = std::move(password); }

    /**
     * @return The pass phrase for the private key.
     */
    auto key_password() const -> const std::optional<std::string>& { return m_password; }

    /**
     * @return The proxy information for this request.
     */
    auto proxy() const -> const std::optional<proxy_data>& { return m_proxy_data; }

    /**
     * Sets proxy information for this request.
     * @param type The type of HTTP proxy to connect to, HTTP or HTTPS.
     * @param host The proxy hostname to connect to.
     * @param port The proxy port to connect to, default=80.
     * @param username The username for authentication with the proxy, default=std::nullopt.
     * @param password The password for authentication with the proxy, default=std::nullopt.
     * @param auth_types The authentication type(s) to use for communication with the proxy.
     *                   If not specified BASIC is used, default=std::nullopt (BASIC).
     */
    auto proxy(
        proxy_type                                 type,
        std::string                                host,
        uint32_t                                   port       = 80,
        std::optional<std::string>                 username   = std::nullopt,
        std::optional<std::string>                 password   = std::nullopt,
        std::optional<std::vector<http_auth_type>> auth_types = std::nullopt) -> void
    {
        m_proxy_data =
            proxy_data{type, std::move(host), port, std::move(username), std::move(password), std::move(auth_types)};
    }

    /**
     * Sets proxy information for this request.
     * @param data The full proxy data to set for this request, @see `proxy_data`.
     */
    auto proxy(proxy_data data) -> void { m_proxy_data = std::move(data); }

    /**
     * @return The list of currently set HTTP Accept-Encoding values.  Note that if set via
     *         `AcceptEndcodingAllAvaliable()` this function will return an empty list.
     */
    auto accept_encodings() const -> const std::optional<std::vector<std::string>>& { return m_accept_encodings; }
    /**
     * IMPORTANT: Using this is mutually exclusive with adding your own Accept-Encoding header.
     * @param encodings A list of accept encodings to send in the request.
     */
    auto accept_encoding(std::optional<std::vector<std::string>> encodings) -> void
    {
        m_accept_encodings = std::move(encodings);
    }

    /**
     * Sets the accept encoding header to all supported encodings that this platform was built with.
     */
    auto accept_encoding_all_available() -> void { m_accept_encodings = std::vector<std::string>{}; }

    /**
     * @return Custom `host:port => ip_addr` resolve hosts for this request.
     */
    auto resolve_hosts() const -> const std::vector<lift::resolve_host>& { return m_resolve_hosts; }

    /**
     * @param resolve_host Adds a resolve host to this request to bypass DNS lookups.
     */
    auto resolve_host(lift::resolve_host resolve_host) -> void
    {
        m_resolve_hosts.emplace_back(std::move(resolve_host));
    }

    /**
     * Clears all set resolve hosts on this request.
     */
    auto clear_resolve_hosts() -> void { m_resolve_hosts.clear(); }

    /**
     * Specifically removes the header from the request.  There are a few
     * default headers that are always added in certain scenarios, this method
     * allows for the user to manually remove them.
     * @param name The name of the header, e.g. 'Accept' or 'Expect'.
     */
    auto remove_header(std::string_view name) -> void { header(name, std::string_view{}); }
    /**
     * Adds a request header with its value.
     * @param name The name of the header, e.g. 'Connection'.
     * @param value The value of the header, e.g. 'Keep-Alive'.
     */
    auto header(std::string_view name, std::string_view value) -> void;

    /**
     * @return The current list of headers added to this request.  Note that if more headers are added
     *         the header classes Name() and Value() string_views might become invalidated.
     */
    auto headers() const -> const std::vector<lift::header>& { return m_request_headers; }

    /**
     * Clears the current set of headers for this request.
     */
    auto clear_headers() -> void { m_request_headers.clear(); }

    /**
     * @return The HTTP body data for this request, if it was never set this will be an empty string.
     */
    auto data() const -> const std::string& { return m_request_data; }

    /**
     * Sets the request to HTTP POST and the body of the request
     * to the provided data.  Override the method after this call to PUT if desired.
     *
     * NOTE: this is mutually exclusive with using mime_field
     * as you cannot include traditional POST data in a mime-type form submission.
     *
     * @param data The request data to send in the HTTP POST.
     * @throw std::logic_error If called after using mime_field().
     */
    auto data(std::string data) -> void;

    /**
     * @return The set mime fields for this request.
     */
    auto mime_fields() const -> const std::vector<lift::mime_field>& { return m_mime_fields; }

    /**
     * @param mf Adds this mime field to this mime HTTP request.
     */
    auto mime_field(lift::mime_field mf) -> void;

    /**
     * https://en.wikipedia.org/wiki/Happy_Eyeballs
     * @param timeout Sets the happy eyeballs algorithm timeout.
     */
    auto happy_eyeballs_timeout(std::chrono::milliseconds timeout) -> void { m_happy_eyeballs_timeout = timeout; }

    /**
     * @return Gets the setting of the happy eyeballs timeout if set.
     */
    auto happy_eyeballs_timeout() const -> const std::optional<std::chrono::milliseconds>&
    {
        return m_happy_eyeballs_timeout;
    }

private:
    /// The on complete handler callback.
    on_complete_handler_type m_on_complete_handler{nullptr};
    /// The transfer progress handler callback.
    transfer_progress_handler_type m_on_transfer_progress_handler{nullptr};
    /// The timeout to connect, or none.
    std::optional<std::chrono::milliseconds> m_connect_timeout{};
    /// The timeout for the request, or none.
    std::optional<std::chrono::milliseconds> m_timeout{};
    /// The timesup for the request, or none.
    std::optional<std::chrono::milliseconds> m_timesup{};
    /// The URL.
    std::string m_url{};
    /// The HTTP request method.
    http::method m_method{http::method::get};
    /// The HTTP version to use for this request.
    http::version m_version{http::version::use_best};
    /// Should this request automatically follow redirects?
    bool m_follow_redirects{true};
    /// How many redirects should be followed? -1 infinite, 0 none, <num>.
    int64_t m_max_redirects{-1};
    /// Should the peer be ssl verified?
    bool m_verify_ssl_peer{true};
    /// Should the host be ssl verified?
    bool m_verify_ssl_host{true};
    /// Should the ssl certificate status be verified?
    bool m_verify_ssl_status{false};
    /// The SSL/TLS certificate file to use.
    std::optional<std::filesystem::path> m_cert_file{};
    /// The SSL/TLS certificate type.
    std::optional<ssl_certificate_type> m_ssl_cert_type{};
    /// The SSL/TLS key file.
    std::optional<std::filesystem::path> m_ssl_key_file{};
    /// The SSL/TLS key file's pass phrase.
    std::optional<std::string> m_password{};
    /// Proxy information.
    std::optional<proxy_data> m_proxy_data{};
    /// Specific Accept-Encoding header fields.
    std::optional<std::vector<std::string>> m_accept_encodings{};
    /// A set of host:port to ip addresses that will be resolved before DNS.
    std::vector<lift::resolve_host> m_resolve_hosts{};
    /// The request headers preformatted into the curl "Header: value\0" format.
    std::vector<lift::header> m_request_headers{};
    /// The POST request body data, mutually exclusive with mime field requests.
    bool        m_request_data_set{false};
    std::string m_request_data{};
    /// The Mime request fields, mutually exclusive with POST request body data.
    bool                          m_mime_fields_set{false};
    std::vector<lift::mime_field> m_mime_fields{};
    /// Happy eyeballs algorithm timeout https://curl.haxx.se/libcurl/c/CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS.html
    std::optional<std::chrono::milliseconds> m_happy_eyeballs_timeout{};

    // libcurl will call this function if the user has requested transfer progress information.
    friend auto curl_xfer_info(
        void*      clientp,
        curl_off_t download_total_bytes,
        curl_off_t download_now_bytes,
        curl_off_t upload_total_bytes,
        curl_off_t upload_now_bytes) -> int;
};

using request_ptr = std::unique_ptr<request>;

} // namespace lift
