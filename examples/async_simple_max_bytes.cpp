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
            << " in " << request->GetTotalTime().count() << " ms with a "
            << "result length of " << request->GetResponseData().length() << std::endl << std::endl;
    }
    else
    {
        std::cout
            << "Error: " << request->GetUrl() << " : "
            << lift::to_string(request->GetCompletionStatus()) << std::endl
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
        "https://upload.wikimedia.org/wikipedia/commons/transcoded/7/76/Nine-spotted_moth_%28Amata_phegea%29_on_a_flower.webm/Nine-spotted_moth_%28Amata_phegea%29_on_a_flower.webm.480p.webm",
        "https://peopledotcom.files.wordpress.com/2017/07/matt-damon.jpg",
        "http://www.example.com",
        "https://www.cnn.com"
    };

    lift::EventLoop event_loop;
    // EventLoops create their own request pools -- grab it to start creating requests.
    auto& request_pool = event_loop.GetRequestPool();

    ssize_t bytes_to_download = 1000;

    /**
     * Create asynchronous requests for each url and inject them into
     * the event loop with 50ms pause between injection
     * and an additional 250ms timeout per each request.
     */
    std::chrono::milliseconds timeout = 550ms;
    for (auto& url : urls)
    {
        std::cout << "Requesting " << url << " to download max byes of : " << bytes_to_download << std::endl;

        lift::Request request = request_pool.Produce(url, on_complete, 0ms);
        request->SetMaxDownloadBytes(bytes_to_download);
        event_loop.StartRequest(std::move(request));
        timeout += 550ms;
        bytes_to_download += 1000;
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
