#include <lift/Lift.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

static auto on_complete(lift::RequestHandle request) -> void
{
    if (request->GetCompletionStatus() == lift::RequestStatus::SUCCESS) {
        std::cout
            << "Completed " << request->GetUrl()
            << " ms:" << request->GetTotalTime().count() << std::endl;
    } else {
        std::cout
            << "Error: " << request->GetUrl() << " : "
            << lift::to_string(request->GetCompletionStatus()) << std::endl;
    }

    /**
     * When the Request destructs here it will return to the pool. To continue working
     * on this request -- or work on the response data in a separate thread std::move()
     * the request to where the processing should be done.
     */
}

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init{};

    std::vector<std::string> urls = {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.reddit.com"
    };

    lift::EventLoop event_loop;
    // EventLoops create their own request pools -- grab it to start creating requests.
    auto& request_pool = event_loop.GetRequestPool();

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 250ms;
    for (auto& url : urls) {
        std::cout << "Requesting " << url << std::endl;
        auto request = request_pool.Produce(url, on_complete, timeout);
        event_loop.StartRequest(std::move(request));
        timeout += 250ms;
        std::this_thread::sleep_for(50ms);
    }

    // Now wait for all the requests to finish before cleaning up.
    while (event_loop.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
