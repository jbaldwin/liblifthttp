#pragma once

#include "AsyncRequest.h"

#include <curl/curl.h>
#include <uv.h>

#include <vector>
#include <mutex>
#include <memory>
#include <list>

class IRequestCallbacks
{
public:
    virtual auto OnHostLookupFailure(std::unique_ptr<AsyncRequest> request) -> void = 0;
    virtual auto OnComplete(std::unique_ptr<AsyncRequest> request) -> void = 0;
    virtual auto OnTimeout(std::unique_ptr<AsyncRequest> request) -> void = 0;
};

class EventLoop
{
public:
    explicit EventLoop(
        std::unique_ptr<IRequestCallbacks> request_callbacks
    );
    ~EventLoop();

    EventLoop(const EventLoop& copy) = delete;
    EventLoop(EventLoop&& move) = default;
    auto operator=(const EventLoop& assign) = delete;
    auto operator=(EventLoop&& assign) -> EventLoop& = default;

    auto Run() -> void;
    auto Stop() -> void;

    auto AddRequest(
        std::unique_ptr<AsyncRequest> curl_request_ptr
    ) -> void;

    auto GetRequestCallbacks() -> IRequestCallbacks&;
    auto GetRequestCallbacks() const -> const IRequestCallbacks&;

private:
    std::unique_ptr<IRequestCallbacks> m_request_callbacks;

    uv_loop_t* m_loop;
    uv_async_t m_async;
    uv_timer_t m_timeout_timer;
    uv_timer_t m_timer2;
    CURLM* m_cmh;
    std::mutex m_pending_requests_lock;
    std::vector<std::unique_ptr<AsyncRequest>> m_pending_requests;
    std::list<std::unique_ptr<AsyncRequest>> m_active_requests;

    auto checkMultiInfo() -> void;

    friend auto curl_start_timeout(
        CURLM* cmh,
        long timeout_ms,
        void* user_data
    ) -> void;

    friend auto curl_handle_socket(
        CURL* curl,
        curl_socket_t socket,
        int action,
        void* user_data,
        void* socketp
    ) -> int;

    friend auto requests_accept_async(
        uv_async_t* async,
        int status
    ) -> void;

    friend auto on_uv_timeout_callback(
        uv_timer_t* handle,
        int status
    ) -> void;

    friend auto on_uv_curl_perform_callback(
        uv_poll_t* req,
        int status,
        int events
    ) -> void;



    friend auto on_uv_timeout_callback_2(
        uv_timer_t* handle,
        int status
    ) -> void;
};
