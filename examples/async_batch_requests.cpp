#include <lift/Lift.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

static auto on_complete(lift::Request request) -> void
{
    switch (request->GetCompletionStatus()) {
    case lift::RequestStatus::SUCCESS:
        std::cout
            << "Completed " << request->GetUrl()
            << " ms:" << request->GetTotalTime().count() << std::endl;
        break;
    case lift::RequestStatus::CONNECT_ERROR:
        std::cout << "Unable to connect to: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::CONNECT_DNS_ERROR:
        std::cout << "Unable to lookup DNS entry for: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::CONNECT_SSL_ERROR:
        std::cout << "SSL Error for: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::TIMEOUT:
        std::cout << "Timeout: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::RESPONSE_EMPTY:
        std::cout << "No response received: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::DOWNLOAD_ERROR:
        std::cout << "Error occurred in CURL write callback: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::ERROR:
        std::cout << "Request had an unrecoverable error: " << request->GetUrl() << std::endl;
        break;
    case lift::RequestStatus::BUILDING:
    case lift::RequestStatus::EXECUTING:
        std::cout
            << "Request is in an invalid state: "
            << to_string(request->GetCompletionStatus()) << std::endl;
        break;
    }
}

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    std::vector<std::string> urls = {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.reddit.com"
    };

    lift::EventLoop event_loop;
    auto& request_pool = event_loop.GetRequestPool();

    {
        std::vector<lift::Request> requests;
        requests.reserve(urls.size());
        for (auto& url : urls) {
            requests.emplace_back(request_pool.Produce(url, on_complete, 250ms));
        }

        /**
         * This will 'move' all of the Request objects into the event loop.
         * The values in the 'requests' vector are now no longer valid.  This
         * example intentionally has 'requests' go out of scope to further
         * demonstrate this.
         */
        event_loop.StartRequests(std::move(requests));
    }

    std::this_thread::sleep_for(100ms); // just to be sure still gets kicked off

    // Now wait for all the requests to finish before cleaning up.
    while (event_loop.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(100ms);
    }

    // Cleanup EventLoop / Threads and LiftHttp library.
    event_loop.Stop();

    lift::cleanup();

    return 0;
}
