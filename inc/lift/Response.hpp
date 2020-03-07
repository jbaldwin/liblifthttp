#pragma once

#include "lift/HeaderView.hpp"
#include "lift/Http.hpp"
#include "lift/RequestStatus.hpp"

#include <curl/curl.h>
#include <uv.h>

#include <chrono>
#include <string>
#include <vector>

namespace lift {
class Request;
class EventLoop;

class Response {
    friend Request;
    friend EventLoop;

public:
    Response() = default;
    ~Response() = default;

    Response(const Response&) = default;
    Response(Response&&) = default;
    auto operator=(const Response&) noexcept -> Response& = default;
    auto operator=(Response&&) noexcept -> Response& = default;

    /**
     * @return The HTTP response status code.
     */
    [[nodiscard]] auto GetResponseStatusCode() const -> http::StatusCode;

    /**
     * @return The HTTP response headers.
     */
    [[nodiscard]] auto GetResponseHeaders() const -> const std::vector<HeaderView>&;

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

private:
    /// The status of this HTTP request.
    RequestStatus m_completions_status{ RequestStatus::BUILDING };
    /// The response headers.
    std::string m_response_headers{};
    /// Views into each header.
    std::vector<HeaderView> m_response_headers_idx{};
    /// The response data if any.
    std::string m_response_data{};

    lift::http::StatusCode m_status_code{ lift::http::StatusCode::HTTP_UNKNOWN };
    std::chrono::milliseconds m_total_time{ 0 };
    uint64_t m_num_connects{ 0 };
    uint64_t m_num_redircts{ 0 };

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
    friend auto on_uv_requests_accept_async(
        uv_async_t* handle) -> void;
};

} // namespace lift
