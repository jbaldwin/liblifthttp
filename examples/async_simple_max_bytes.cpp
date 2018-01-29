#include <lift/Lift.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

static auto on_complete(lift::Request request) -> void
{
    if(request->GetCompletionStatus() == lift::RequestStatus::SUCCESS)
    {
        std::cout
            << "Completed " << request->GetUrl()
            << " ms:" << request->GetTotalTimeMilliseconds() << std::endl
            << "Result length: " << request->GetResponseData().length() << std::endl
            << "Total bytes received: " << request->GetTotalBytesReceived() << std::endl << std::endl;
    }
    else
    {
        std::cout
            << "Error: " << request->GetUrl() << " : "
            << lift::request_status2str(request->GetCompletionStatus()) << std::endl
            << "Result length: " << request->GetResponseData().length() << std::endl << std::endl;
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
    lift::initialize();

    std::vector<std::string> urls =
    {
        "http://www.example.com",
        "https://www.google.com",
        "https://www.reddit.com",
        "http://www.sample-videos.com/video/mp4/720/big_buck_bunny_720p_10mb.mp4",
        "https://peopledotcom.files.wordpress.com/2017/07/matt-damon.jpg"
    };

    lift::EventLoop event_loop;
    // EventLoops create their own request pools -- grab it to start creating requests.
    auto& request_pool = event_loop.GetRequestPool();

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 5500ms;
    for(auto& url : urls)
    {
        std::cout << "Requesting " << url << std::endl;
        lift::Request request = request_pool.Produce(url, on_complete, 0ms, 5000);
        event_loop.StartRequest(std::move(request));
        timeout += 550ms;
        std::this_thread::sleep_for(50ms);
    }

    std::cout << std::endl;


    // Now wait for all the requests to finish before cleaning up.
    while(event_loop.GetActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(100ms);
    }

    // Cleanup EventLoop / Threads and LiftHttp library.
    event_loop.Stop();

    lift::cleanup();

    return 0;
}
