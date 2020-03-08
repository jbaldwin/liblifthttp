#pragma once

#include "lift/HeaderView.hpp"
#include "lift/Http.hpp"
#include "lift/LiftStatus.hpp"

#include <curl/curl.h>
#include <uv.h>

#include <chrono>
#include <string>
#include <vector>

namespace lift {
class EventLoop;
class Executor;

class Response {
    friend EventLoop;
    friend Executor;

public:
    Response();
    ~Response() = default;

    Response(const Response&) = default;
    Response(Response&&) = default;
    auto operator=(const Response&) noexcept -> Response& = default;
    auto operator=(Response&&) noexcept -> Response& = default;

    /**
     * The Lift status is how the request ended up in the event loop.
     * It might have completed successfully, or timed out, or had an SSL error, etc.
     *
     * This is not the HTTP status code returned by the remote server.
     *
     * @return Gets the request completion status.
     */
    [[nodiscard]] auto LiftStatus() const -> lift::LiftStatus { return m_lift_status; }

    /**
     * @return The HTTP response status code.
     */
    [[nodiscard]] auto StatusCode() const -> http::StatusCode { return m_status_code; }

    /**
     * @return The HTTP response headers.
     */
    [[nodiscard]] auto Headers() const -> const std::vector<HeaderView>& { return m_headers_idx; }

    /**
     * @return The HTTP download payload.
     */
    [[nodiscard]] auto Data() const -> const std::string& { return m_data; }

    /**
     * @return The total HTTP request time in milliseconds.
     */
    [[nodiscard]] auto TotalTime() const -> std::chrono::milliseconds { return m_total_time; }

    /**
     * @return The number of connections made to make this request
     */
    [[nodiscard]] auto NumConnects() const -> uint64_t { return m_num_connects; }

    /**
     * @return The number of redirects made during this request.
     */
    [[nodiscard]] auto NumRedirects() const -> uint64_t { return m_num_redircts; }

private:
    /// The status of this HTTP request.
    lift::LiftStatus m_lift_status{ lift::LiftStatus::BUILDING };
    /// The response headers.
    std::string m_headers{};
    /// Views into each header.
    std::vector<HeaderView> m_headers_idx{};
    /// The response data if any.
    std::string m_data{};
    /// The HTTP response status code.
    lift::http::StatusCode m_status_code{ lift::http::StatusCode::HTTP_UNKNOWN };
    /// The total time in milliseconds to execute the request.
    std::chrono::milliseconds m_total_time{ 0 };
    /// The number of times attempted to connect to the remote server.
    uint64_t m_num_connects{ 0 };
    /// The number of redirects traversed while processing the request.
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

    /// libuv will call this function when the StartRequest() function is called.
    friend auto on_uv_requests_accept_async(
        uv_async_t* handle) -> void;
};

} // namespace lift
