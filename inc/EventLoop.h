#pragma once

#include <curl/curl.h>
#include <uv.h>

class EventLoop
{
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop& copy) = delete;
    EventLoop(EventLoop&& move) = default;
    auto operator=(const EventLoop& assign) = delete;

    auto Run() -> void;

    auto GetUVLoop() -> uv_loop_t*;
    auto GetUVTimeoutTimer() -> uv_timer_t*;
    auto GetCurlMultiHandle() -> CURLM*;

    auto OnUvTimeout(uv_timer_t* handle, int status);
    auto CurlPerform(
        uv_poll_t* req,
        int status,
        int events
    ) -> void;

private:
    uv_loop_t* m_loop;
    uv_timer_t m_timeout_timer;
    CURLM* m_cmh;

    auto checkMultiInfo() -> void;
};
