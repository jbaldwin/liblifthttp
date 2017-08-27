#include "Request.h"
#include "EventLoop.h"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>

class Driver : public IRequestCallbacks
{
public:
    uint64_t m_completed = 2;

    auto OnHostLookupFailure(std::unique_ptr<AsyncRequest> request) -> void override
    {
        std::cout << "HOST LOOKUP FAILURE " << request->GetUrl() << std::endl;
        m_completed--;
    }

    auto OnComplete(std::unique_ptr<AsyncRequest> request) -> void override
    {
        std::cout << "COMPLETED " << request->GetUrl() << " ms:" << request->GetTotalTimeMilliseconds() << std::endl;
//        std::cout << request->GetDownloadData() << std::endl;
        m_completed--;
    }

    auto OnTimeout(std::unique_ptr<AsyncRequest> request) -> void override
    {
        std::cout << request->GetUrl() << " timedout..." << std::endl;
        m_completed--;
    };
};

auto run_driver(EventLoop& event_loop) -> void
{
    std::cout << "run_driver start" << std::endl;

    event_loop.Run();

    std::cout << "run_driver stop" << std::endl;
}

int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);

//    Request request("http://www.example.com");
//
//    request.Perform();
//    std::cout << request.GetDownloadData() << std::endl << std::endl;

    auto driver_ptr = std::make_unique<Driver>();
    EventLoop event_loop(std::move(driver_ptr));

    std::thread driver_thread(run_driver, std::ref(event_loop));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);

    auto async_request_ptr1 = std::make_unique<AsyncRequest>("http://www.cnn.com");
    async_request_ptr1->SetTimeoutMilliseconds(50);
    event_loop.AddRequest(std::move(async_request_ptr1));

    auto async_request_ptr2 = std::make_unique<AsyncRequest>("http://www.example.com");
    async_request_ptr2->SetTimeoutMilliseconds(50);
    event_loop.AddRequest(std::move(async_request_ptr2));

    while(true)
    {
        const auto& driver = static_cast<Driver&>(event_loop.GetRequestCallbacks());
        if(driver.m_completed == 0)
        {
            std::cout << "driver.m_completed == 0 breaking" << std::endl;
            break;
        }
        std::this_thread::sleep_for(1s);
    }

    std::cout << "Stopping event loop " << std::endl;
    event_loop.Stop();
    std::cout << "Joining driver threads " << std::endl;
    driver_thread.join();

    std::cout << "Cleaning up curl " << std::endl;

    curl_global_cleanup();

    return 0;
}
