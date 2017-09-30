#include "Lift.h"

#include <atomic>

namespace lift
{

auto initialize() -> void
{
    static std::atomic<uint64_t> initialized{0};

    if(initialized.fetch_add(1) == 0)
    {
        curl_global_init(CURL_GLOBAL_ALL);
    }
}

auto cleanup() -> void
{
    static std::atomic<uint64_t> cleaned{0};

    if(cleaned.fetch_add(1) == 0)
    {
        curl_global_cleanup();
    }
}

} // lift
