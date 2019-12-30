#pragma once

#include "lift/Request.hpp"
#include "lift/RequestHandle.hpp"
#include "lift/ResolveHost.hpp"

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>

namespace lift {

class CurlPool;

class RequestPool {
    friend class RequestHandle; // for returning into the pool
    friend class Request; // for resolve hosts

public:
    /**
     * @param resolve_hosts A set of host:port combinations to bypass DNS resolving.
     */
    explicit RequestPool(
        std::vector<ResolveHost> resolve_hosts = {});
    ~RequestPool() = default;

    RequestPool(const RequestPool&) = delete;
    RequestPool(RequestPool&&) = delete;
    auto operator=(const RequestPool&) -> RequestPool& = delete;
    auto operator=(RequestPool &&) -> RequestPool& = delete;

    /**
     * This can be useful to pre-allocate a large number of Request objects to be used
     * immediately and incur expensive start up (like CURL* handles) before starting requests.
     * @param count Pre-allocates/reserves this many request objects.
     */
    auto Reserve(
        size_t count) -> void;

    /**
     * Produces a new Request with the specified timeout.
     *
     * This produce method is best used for synchronous requests.
     *
     * @param url The url of the Request.
     * @param timeout The timeout of the request, a timeout of 0 means infinite.
     * @return A Request object setup for the URL + Timeout.
     */
    auto Produce(
        const std::string& url,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{ 0 }) -> RequestHandle;

    /**
     * Produces a new Request.  This function is thread safe.
     *
     * This produce method is best used for asynchronous requests.
     *
     * @param url The url of the Request.
     * @param on_complete_handler The on completion callback handler for this request.
     * @param timeout The timeout of the request.
     * @return A Request object setup for the URL + Timeout.
     */
    auto Produce(
        const std::string& url,
        std::function<void(RequestHandle)> on_complete_handler,
        std::chrono::milliseconds timeout) -> RequestHandle;

    /**
     * @return The set of host:port combinations that bypass DNS lookup,
     *         all requests that are produced from this pool will have this
     *         set of resolve hosts automatically applied.
     */
    [[nodiscard]] auto GetPoolResolveHosts() const noexcept -> const std::vector<ResolveHost>&;

private:
    /// Used for thread safe calls.
    std::mutex m_lock{};
    /// Pool of un-used Request handles.
    std::deque<std::unique_ptr<Request>> m_requests{};
    /// The set of resolve hosts to apply to all requests in this pool.
    std::vector<ResolveHost> m_resolve_hosts{};

    /**
     * Returns a Request object to the pool to be re-used.
     *
     * This function is thread safe.
     *
     * @param request_ptr The request to return to the pool to be re-used.
     */
    auto returnRequest(
        std::unique_ptr<Request> request_ptr) -> void;
};

} // lift
