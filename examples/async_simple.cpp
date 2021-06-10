#include <lift/lift.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

static auto on_complete(lift::request_ptr request_ptr, lift::response response) -> void
{
    if (response.lift_status() == lift::lift_status::success)
    {
        std::cout << "Completed " << request_ptr->url() << " ms:" << response.total_time().count() << std::endl;
    }
    else
    {
        std::cout << "Error: " << request_ptr->url() << " : " << lift::to_string(response.lift_status()) << std::endl;
    }
}

int main()
{
    using namespace std::chrono_literals;

    std::vector<std::string> urls = {"http://www.example.com", "http://www.google.com", "http://www.reddit.com"};

    lift::client client{};

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 250ms;
    for (auto& url : urls)
    {
        std::cout << "Requesting " << url << std::endl;
        auto request_ptr = std::make_unique<lift::request>(url, timeout);
        client.start_request(std::move(request_ptr), on_complete);
        timeout += 250ms;
        std::this_thread::sleep_for(50ms);
    }

    // Now wait for all the requests to finish before cleaning up.
    while (!client.empty())
    {
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
