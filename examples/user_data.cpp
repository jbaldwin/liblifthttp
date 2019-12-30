#include <lift/Lift.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static auto on_complete(lift::RequestHandle request, uint64_t user_data_value1, double user_data_value2) -> void
{
    std::cout << "RequestHandle id " << user_data_value1 << " with double " << user_data_value2 << " has completed: " << request->GetUrl() << std::endl;
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    using namespace std::chrono_literals;

    lift::GlobalScopeInitializer lift_init{};

    lift::EventLoop event_loop{};
    auto& request_pool = event_loop.GetRequestPool();
    auto req1 = request_pool.Produce("http://www.example.com", 1s);
    req1->SetOnCompleteHandler([](lift::RequestHandle r) { on_complete(std::move(r), 1, 100.5); });
    event_loop.StartRequest(std::move(req1));

    auto req2 = request_pool.Produce("http://www.reddit.com", 1s);
    req2->SetOnCompleteHandler([](lift::RequestHandle r) { on_complete(std::move(r), 2, 1234.567); });
    event_loop.StartRequest(std::move(req2));

    // sleep for a bit so this thread doesn't grab the active request count too fast and just shutdown
    std::this_thread::sleep_for(500ms);

    while (event_loop.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(10ms);
    }

    return 0;
}
