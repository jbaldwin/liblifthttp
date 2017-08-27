#pragma once

#include "Request.h"

#include <list>

class AsyncRequest : public Request
{
    friend class EventLoop;

public:
    AsyncRequest();
    AsyncRequest(const std::string& url);

    ~AsyncRequest() override;

    AsyncRequest(const AsyncRequest& copy) = delete;
    AsyncRequest(AsyncRequest&& move) = default;
    auto operator=(const AsyncRequest& assign) = delete;
    auto operator=(AsyncRequest&& assign) -> AsyncRequest& = default;

private:
    std::list<std::unique_ptr<AsyncRequest>>::iterator m_active_requests_position;

    friend auto requests_accept_async(uv_async_t* async, int status) -> void;
};
