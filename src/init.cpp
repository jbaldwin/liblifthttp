#include "lift/init.hpp"

namespace lift
{
std::mutex            g_lift_mutex{};
std::atomic<uint64_t> g_lift_init{0};

} // namespace lift
