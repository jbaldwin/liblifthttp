#pragma once

#include "lift/request.hpp"
#include "lift/response.hpp"

#include <curl/curl.h>

#include <map>

namespace lift
{
class request;
class client;

/**
 * This class's design is to encapsulate executing either a synchronous
 * or asynchronous request while also maintaining ownership boundaries
 * between the user of the liblifthttp library and the CURL objects.
 * A previous design exposed the CURL* as well as CURLM* objects
 * behind well formed C++ objects, unfortunately the user could
 * mutate these CURL objects at certain points during execution
 * and cause 'bad things to happen'.  By splitting the request and
 * response objects out from any underlying CURL objects the lifetimes
 * can be appropriately managed.
 *
 * This class should never be used directly by the user of liblifthttp,
 * hence it has no public methods other than its destructor.
 */
class executor
{
    /// Allowed to create executor and Timesup!
    friend client;
    /// Allowed to create executors.
    friend request;

public:
    executor(const executor&)                    = delete;
    executor(executor&&)                         = delete;
    auto operator=(const executor&) -> executor& = delete;
    auto operator=(executor&&) -> executor&      = delete;
    ~executor();

private:
    /// The curl handle to execute against.
    CURL* m_curl_handle{curl_easy_init()};
    /// The mime handle if present.
    curl_mime* m_mime_handle{nullptr};
    /// The HTTP curl request headers.
    curl_slist* m_curl_request_headers{nullptr};
    /// The HTTP curl resolve hosts.
    curl_slist* m_curl_resolve_hosts{nullptr};

    /// If sync request the pointer to the request.
    request* m_request_sync{nullptr};

    /// If async request the client executing this request.
    client* m_client{nullptr};
    /// If async request the pointer to the request.
    request_ptr m_request_async{nullptr};
    /// If the async request has a timeout set then this is the position to delete when completed.
    std::optional<std::multimap<uint64_t, executor*>::iterator> m_timeout_iterator{};
    // Has the on complete handler already been processed?
    bool m_on_complete_handler_processed{false};

    /// Used internally to point at one of the sync or async requests.
    request* m_request{nullptr};

    /// The HTTP response data.
    response m_response{};

    static auto make_unique(client* c) -> std::unique_ptr<executor>
    {
        return std::unique_ptr<executor>(new executor{c});
    }

    /**
     * This constructor is used for executing a synchronous request.
     * @param request The synchronous request pointer.
     */
    executor(request* request);

    /**
     * This constructor is used for executing an asynchronous requests.
     * @param c The client that will execute this request.
     */
    executor(client* c);

    /**
     * @param req_ptr The asynchronous request to execute.
     */
    auto start_async(request_ptr req_ptr) -> void;

    /**
     * Synchronously performs the request and returns the response.
     * @return The HTTP response.
     */
    auto perform() -> response;

    /**
     * Prepares the request to be executed.  This will setup all required
     * curl_easy_setopt() calls based on what has been set on the lift::request.
     */
    auto prepare() -> void;

    /**
     * Copies all available HTTP response fields into the lift::response from
     * the curl handle.
     * @param curl_code The result of the curl_easy_perform call.
     */
    auto copy_curl_to_response(CURLcode curl_code) -> void;

    /**
     * Sets the response object with appropriate times up values.
     * @param total_time The total time the request executed for.
     */
    auto set_timesup_response(std::chrono::milliseconds total_time) -> void;

    auto reset() -> void;

    /**
     * Converts a CURLcode into a lift_status.
     * @param curl_code The CURLcode to convert.
     */
    static auto convert(CURLcode curl_code) -> lift_status;

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

    friend auto on_uv_requests_accept_async(uv_async_t* handle) -> void;

    /// For Timesup.
    friend auto on_uv_timesup_callback(uv_timer_t* handle) -> void;

    /// libcurl will call this function when the request has debug function enabled.
    friend auto curl_debug_info_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
        -> int;
};

using executor_ptr = std::unique_ptr<executor>;

} // namespace lift
