#pragma once

#include "lift/header.hpp"
#include "lift/http.hpp"
#include "lift/lift_status.hpp"

#include <curl/curl.h>
#include <uv.h>

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace lift
{
class client;
class executor;

class response
{
    friend client;
    friend executor;

public:
    response();
    ~response() = default;

    response(const response&)                             = default;
    response(response&&)                                  = default;
    auto operator=(const response&) noexcept -> response& = default;
    auto operator=(response&&) noexcept -> response&      = default;

    /**
     * The Lift status is how the request ended up in the event loop.
     * It might have completed successfully, or timed out, or had an SSL error, etc.
     *
     * This is not the HTTP status code returned by the remote server.
     *
     * @return Gets the request completion status.
     */
    [[nodiscard]] auto lift_status() const -> lift::lift_status { return m_lift_status; }

    /**
     * @return The HTTP version of the response.
     */
    auto version() const -> http::version { return m_version; }

    /**
     * @return The HTTP response status code.
     */
    [[nodiscard]] auto status_code() const -> http::status_code { return m_status_code; }

    /**
     * @return The HTTP response headers.
     */
    [[nodiscard]] auto headers() const -> const std::vector<header>& { return m_headers; }

    /**
     * @return The header if it exists on this response, otherwise std::nullopt.
     */
    [[nodiscard]] auto header(std::string_view name) const -> std::optional<std::reference_wrapper<const lift::header>>;

    /**
     * @return The HTTP download payload.
     */
    [[nodiscard]] auto data() const -> std::string_view { return std::string_view{m_data.data(), m_data.size()}; }

    /**
     * @return The total HTTP request time in milliseconds.
     */
    [[nodiscard]] auto total_time() const -> std::chrono::milliseconds
    {
        return std::chrono::milliseconds{m_total_time};
    }

    /**
     * This feature has to be enabled by calling the request::enable_error_message() method.
     * 
     * @return The network error message for diagnostics in case of a network request failure (if enabled).
     */
    [[nodiscard]] const std::optional<std::string>& network_error_message() const
    {
        return m_network_error_message;
    }

    /**
     * @return The number of connections made to make this request
     */
    [[nodiscard]] auto num_connects() const -> uint8_t { return m_num_connects; }

    /**
     * @return The number of redirects made during this request.
     */
    [[nodiscard]] auto num_redirects() const -> uint8_t { return m_num_redirects; }

    /**
     * Formats the response in the raw HTTP format.
     */
    friend auto operator<<(std::ostream& os, const response& r) -> std::ostream&;

private:
    /// Ordered by sizeof() since response gets std::moved()'ed back to the client.

    /// The response headers.
    std::vector<lift::header> m_headers{};
    /// The response data if any.
    std::vector<char> m_data{};
    /// The total time in milliseconds to execute the request, stored as uint32_t since that is enough
    /// time for 49~ days and saves 4 bytes from std::chrono::milliseconds.
    uint32_t m_total_time{0};
    /// The HTTP response status code.
    lift::http::status_code m_status_code{lift::http::status_code::http_unknown};
    /// The status of this HTTP request.
    lift::lift_status m_lift_status{lift::lift_status::building};
    /// The HTTP response version.
    http::version m_version{http::version::v1_1};
    /// The number of times attempted to connect to the remote server.
    uint8_t m_num_connects{0};
    /// The number of redirects traversed while processing the request.
    uint8_t m_num_redirects{0};
    // The network error message for diagnostics in case of a network request failure, std::nullopt otherwise.
    std::optional<std::string> m_network_error_message{std::nullopt};

    /// libcurl will call this function when a header is received for the HTTP request.
    friend auto curl_write_header(char* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t;

    /// libcurl will call this function when data is received for the HTTP request.
    friend auto curl_write_data(void* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t;

    /// libcurl will call this function if the user has requested transfer progress information.
    friend auto curl_xfer_info(
        void*      clientp,
        curl_off_t download_total_bytes,
        curl_off_t download_now_bytes,
        curl_off_t upload_total_bytes,
        curl_off_t upload_now_bytes) -> int;

    /// libuv will call this function when the start_request() function is called.
    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

    /// For Timesup.
    friend auto on_uv_timesup_callback(uv_timer_t* handle) -> void;
};

} // namespace lift
