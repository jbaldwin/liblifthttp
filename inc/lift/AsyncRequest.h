#pragma once

#include "lift/Request.h"

#include <list>

namespace lift
{

class AsyncRequest : public Request
{
    friend class EventLoop;

public:
    AsyncRequest();

    explicit AsyncRequest(
        const std::string& url,
        uint64_t timeout_ms = 0
    );

    ~AsyncRequest() override = default;

    /**
     * @param copy No copying allowed.
     */
    AsyncRequest(const AsyncRequest& copy) = delete;

    /**
     * @param move Moving is allowed.
     */
    AsyncRequest(AsyncRequest&& move) = default;

    /**
     *
     * @param assign No copy assignment allowed.
     */
    auto operator=(const AsyncRequest& assign) -> AsyncRequest = delete;

    /**
     * @param assign Move assigned allowed.
     */
    auto operator=(AsyncRequest&& assign) -> AsyncRequest& = default;

private:
    std::list<std::unique_ptr<AsyncRequest>>::iterator m_active_requests_position; ///< The event loop list position for this request.  [internal]

    /**
     * On completion of the request the EventLoop will call this to set the appropriate status
     * of the async request.
     * @param curl_request_code The curl completion status of the HTTP request.
     */
    auto setRequestStatus(
        CURLcode curl_request_code
    ) -> void;

    friend auto requests_accept_async(
        uv_async_t* async,
        int status
    ) -> void; ///< Friend for access to m_active_requests_position within the EventLoop.
};

} // lift
