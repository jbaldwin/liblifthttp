#pragma once

#include <atomic>
#include <curl/curl.h>
#include <mutex>

namespace lift {

extern std::mutex g_lift_mutex;
extern std::atomic<uint64_t> g_lift_init;

inline auto global_init() -> void
{
    if (g_lift_init.fetch_add(1) == 0) {
        std::lock_guard<std::mutex> g { g_lift_mutex };
        curl_global_init(CURL_GLOBAL_ALL);
    }
}

inline auto global_cleanup() -> void
{
    if (g_lift_init.fetch_sub(1) == 1) {
        std::lock_guard<std::mutex> g { g_lift_mutex };
        curl_global_cleanup();
    }
}

} // lift