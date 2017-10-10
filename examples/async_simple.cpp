#include <lift/Lift.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

class CompletedCtx : public lift::IRequestCallback
{
public:
    CompletedCtx(
        size_t total_requests
    )
        : m_completed(total_requests)
    {

    }

    std::atomic<size_t> m_completed;    ///< This variable signals to the main thread all the requests are completed.

    auto OnComplete(lift::Request request) -> void override
    {
        m_completed--;
        switch(request->GetCompletionStatus())
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
                          << request_status2str(request->GetCompletionStatus()) << std::endl;
                break;
        }

        /**
         * When the Request destructs here it will return to the pool. To continue working
         * on this request -- or work on the response data in a separate thread std::move()
         * the request to where the processing should be done.
         */
    }
};

int main(int argc, char* argv[])
{
    using namespace std::chrono_literals;
    (void)argc;
    (void)argv;

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    std::vector<std::string> urls =
    {
        "http://www.example.com",
        "http://www.google.com",
        "http://www.reddit.com"
    };

    // Create the EventLoop with a Request callback 'Completed'.
    lift::EventLoop event_loop(std::make_unique<CompletedCtx>(urls.size()));
    auto& request_pool = event_loop.GetRequestPool();

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 250ms;
    for(auto& url : urls)
    {
        std::cout << "Requesting " << url << std::endl;
        lift::Request request = request_pool.Produce(url, timeout);
        event_loop.AddRequest(std::move(request));
        timeout += 250ms;
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

    lift::cleanup();

    return 0;
}
