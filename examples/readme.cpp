#include <iostream>
#include <lift/lift.hpp>

int main()
{
    // Synchronous requests can be created on the stack.
    lift::request request{"http://www.example.com"};
    // This is the blocking synchronous HTTP call.
    auto response = request.perform();
    std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
    std::cout << response << "\n\n"; // Will print the raw http response.

    // Creating the client starts it immediately, it spawns a background thread for executing requests.
    lift::client client{};

    // Create the request just like we did in the sync version, but now provide a lambda for on completion.
    // NOTE: The Lambda is executed ON the Lift client background thread, not this main thread!
    //       Use the std::promise + std::future async below if you want to pass ownership back to
    //       this thread.
    auto request_ptr = lift::request::make_unique(
        "http://www.example.com",
        std::chrono::seconds{10}, // Give the request 10 seconds to complete or timeout.
        [](lift::request_ptr req_ptr, lift::response response) {
            std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
            std::cout << response << "\n\n";
        });

    // Create a second async request that works via a promise+future instead of a functor callback.
    auto request_with_promise_ptr = lift::request::make_unique("http://www.example.com", std::chrono::seconds{10});
    // Grab the requests future before submitting it into the lift client.
    auto future = request_with_promise_ptr->async_future();

    // Now inject the two async requests into the client to be executed.  Moving into the client is required,
    // this passes ownership of the request to the client's background worker thread.
    client.start_request(std::move(request_ptr));
    client.start_request(std::move(request_with_promise_ptr));

    // Block until the async request completes, this returns the original request and the response.
    auto [req_ptr, resp] = future.get();
    std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
    std::cout << response << "\n\n"; // Will print the raw http response.

    // Block on this main thread until the lift client has completed the request, or timed out.
    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    return 0;
}
