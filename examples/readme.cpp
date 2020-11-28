#include <iostream>
#include <lift/lift.hpp>

int main()
{
    // Synchronous requests can be created on the stack.
    lift::request request{"http://www.example.com"};
    // This is the blocking synchronous HTTP call.
    auto response = request.perform();
    std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
    std::cout << response << "\n"; // Will print the raw http response.

    // Creating the client starts it immediately, it spawns a background thread for executing requests.
    lift::client client{};

    // Create the request just like we did in the sync version, but now provide a lambda for on completion.
    // NOTE: that the Lambda is executed ON the Lift client background thread.  If you want to handle
    // on completion processing on this main thread you need to std::move() it back via a queue or inter-thread
    // communication.  This is imporant if any resources are shared between the threads.
    // NOTE: The request is created on the heap so ownership can be passed easily via an std::unique_ptr
    // to the lift::client!  lift::request::make_unique() is a handy function to easily do so.
    auto request_ptr = lift::request::make_unique(
        "http://www.example.com",
        std::chrono::seconds{10}, // Give the request 10 seconds to complete or timeout.
        [](lift::request_ptr req_ptr, lift::response response) {
            std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
            std::cout << response << "\n";
        });

    // Now inject the request into the client to be executed.  Moving into the client is required,
    // this passes ownership of the request to the client's background worker thread.
    client.start_request(std::move(request_ptr));

    // Block on this main thread until the lift client has completed the request, or timed out.
    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    return 0;
}
