#include <lift/Lift.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

static auto on_complete(std::unique_ptr<lift::Request> request_ptr, lift::Response response) -> void
{
    if (response.LiftStatus() == lift::LiftStatus::SUCCESS)
    {
        std::cout << "Completed " << request_ptr->Url() << " ms:" << response.TotalTime().count() << std::endl;
    }
    else
    {
        std::cout << "Error: " << request_ptr->Url() << " : " << lift::to_string(response.LiftStatus()) << std::endl;
    }
}

int main()
{
    using namespace std::chrono_literals;

    std::vector<std::string> urls = {"http://www.example.com", "http://www.google.com", "http://www.reddit.com"};

    lift::EventLoop event_loop{};

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 250ms;
    for (auto& url : urls)
    {
        std::cout << "Requesting " << url << std::endl;
        auto request_ptr = lift::Request::make_unique(url, timeout, on_complete);
        event_loop.StartRequest(std::move(request_ptr));
        timeout += 250ms;
        std::this_thread::sleep_for(50ms);
    }

    // Now wait for all the requests to finish before cleaning up.
    while (event_loop.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
