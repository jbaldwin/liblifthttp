#include <lift/Lift.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

class CompletedCtx : public lift::IRequestCb
{
public:
    CompletedCtx(
        size_t total_requests,
        lift::RequestPool& request_pool
    )
        :
            m_completed(total_requests),
            m_request_pool(request_pool)
    {

    }

    std::atomic<size_t> m_completed;    ///< This variable signals to the main thread all the requests are completed.
    lift::RequestPool&  m_request_pool; ///< This reference is used to re-use Request objects.

    auto OnComplete(std::unique_ptr<lift::Request> request) -> void override
    {
        m_completed--;
        switch(request->GetStatus())
        {
            case lift::RequestStatus::SUCCESS:
                std::cout
                    << "Completed " << request->GetUrl()
                    << " ms:" << request->GetTotalTimeMilliseconds() << std::endl << std::endl;

                for(auto& header : request->GetResponseHeaders())
                {
                    std::cout << header.GetName();
                    if(header.HasValue())
                    {
                        std::cout << ": " << header.GetValue();
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl << request->GetResponseData() << std::endl << std::endl;
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
            case lift::RequestStatus::BUILDING:
            case lift::RequestStatus::EXECUTING:
                std::cout << "Request is in an invalid state: "
                          << request_status2str(request->GetStatus()) << std::endl;
                break;
        }

        /**
         * This return crosses thread barriers, but it is safe as the RequestPool
         * internally is thread safe.
         */
        m_request_pool.Return(std::move(request));
    }
};

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    lift::RequestPool request_pool;
    std::vector<std::string> urls =
    {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.reddit.com"
    };

    // Create the EventLoop with a Request callback 'Completed'.
    lift::EventLoop event_loop(std::make_unique<CompletedCtx>(urls.size(), request_pool));

    // Spin-up a separate thread to run the asynchronous requests and drive the event loop.
    std::thread driver_thread(
        [&]() { event_loop.Run(); }
    );

    // Wait for the thread to spin-up and run the event loop.
    while(!event_loop.IsRunning())
    {
        std::this_thread::sleep_for(100ms);
    }

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    uint64_t timeout_ms = 250;
    for(auto& url : urls)
    {
        auto request = request_pool.Produce(url, timeout_ms);
        event_loop.AddRequest(std::move(request));
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
