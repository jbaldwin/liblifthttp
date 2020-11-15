#include <iostream>
#include <lift/Lift.hpp>

int main()
{
    // Synchronous requests can be created on the stack.
    lift::Request request{"http://www.example.com"};
    // This is the blocking synchronous HTTP call.
    auto response = request.Perform();
    std::cout << "LiftStatus: " << lift::to_string(response.LiftStatus()) << "\n";
    std::cout << "HTTP Status Code: " << lift::http::to_string(response.StatusCode()) << "\n";
    for (const auto& header : response.Headers())
    {
        std::cout << header.Name() << ": " << header.Value() << "\n";
    }
    std::cout << response.Data();

    // Creating the event loop starts it immediately, it spawns a background thread for executing requests.
    lift::EventLoop loop{};

    // Create the request just like we did in the sync version, but now provide a lambda for on completion.
    // NOTE: that the Lambda is executed ON the Lift event loop background thread.  If you want to handle
    // on completion processing on this main thread you need to std::move() it back via a queue or inter-thread
    // communication.  This is imporant if any resources are shared between the threads.
    // NOTE: The request is created on the heap so ownership can be passed easily via an std::unique_ptr
    // to the lift::EventLoop!  lift::Request::make_unique() is a handy function to easily do so.
    auto request_ptr = lift::Request::make_unique(
        "http://www.example.com",
        std::chrono::seconds{10}, // Give the request 10 seconds to complete or timeout.
        [](lift::RequestPtr req_ptr, lift::Response response) {
            std::cout << "LiftStatus: " << lift::to_string(response.LiftStatus()) << "\n";
            std::cout << "HTTP Status Code: " << lift::http::to_string(response.StatusCode()) << "\n";
            for (const auto& header : response.Headers())
            {
                std::cout << header.Name() << ": " << header.Value() << "\n";
            }
            std::cout << response.Data();
        });

    // Now inject the request into the event to be executed.  Moving into the event loop is required,
    // this passes ownership of the request to the event loop background worker thread.
    loop.StartRequest(std::move(request_ptr));

    // Block on this main thread until the lift event loop background thread has completed the request, or timed out.
    while (loop.ActiveRequestCount() > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    return 0;
}
