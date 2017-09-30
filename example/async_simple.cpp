#include "Lift.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

class CompletedCtx : public lift::IRequestCb
{
public:
    // This variable is used across the Completed context and main threads.
    std::atomic<uint64_t> m_completed{3};

    auto OnComplete(std::unique_ptr<lift::AsyncRequest> request) -> void override
    {
        m_completed--;
        switch(request->GetStatus())
        {
            case lift::RequestStatus::SUCCESS:
                std::cout
                    << "COMPLETED " << request->GetUrl()
                    << " ms:" << request->GetTotalTimeMilliseconds() << std::endl
                    << request->GetDownloadData() << std::endl << std::endl;
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
            case lift::RequestStatus::ERROR:
                std::cout << "Request had an unrecoverable error: " << request->GetUrl() << std::endl;
                break;
        }
    }
};

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();
    // Create the EventLoop with a Request callback 'Completed'.
    lift::EventLoop event_loop(std::make_unique<CompletedCtx>());

    // Spin-up a separate thread to run the asynchronous requests and drive the event loop.
    std::thread driver_thread(
        [&]() { event_loop.Run(); }
    );

    // Wait for the thread to spin-up and run the event loop.
    while(!event_loop.IsRunning())
    {
        std::this_thread::sleep_for(100ms);
    }

    auto urls = std::vector<std::string>
        {
            "http://www.example.com",
            "http://www.google.com",
            "http://www.reddit.com"
        };

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    uint64_t timeout_ms = 250;
    for(auto& url : urls)
    {
        auto async_request = std::make_unique<lift::AsyncRequest>(url, timeout_ms);
        event_loop.AddRequest(std::move(async_request));
        timeout_ms += 250;
        std::this_thread::sleep_for(50ms);
    }

    // Now wait for all the requests to finish before cleaning up.
    const auto& completed_ctx = static_cast<CompletedCtx&>(event_loop.GetRequestCallback());
    while(completed_ctx.m_completed > 0)
    {
        std::this_thread::sleep_for(100ms);
    }

    // Cleanup EventLoop / Threads and LiftHttp library.
    event_loop.Stop();
    driver_thread.join();

    lift::cleanup();

    return 0;
}
