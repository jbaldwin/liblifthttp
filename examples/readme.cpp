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
    // The lambda is provided when the request is started on the client.
    // NOTE: The Lambda is executed ON the Lift client background thread, not this main thread!
    //       Use the std::promise + std::future async below if you want to pass ownership back to
    //       this thread.
    // This request is given 10 seconds to complete or timeout
    auto request_with_callback_ptr = lift::request::make_unique("http://www.example.com", std::chrono::seconds{10});

    // Create a second async request that works via a promise+future instead of a functor callback.
    auto request_with_future_ptr = lift::request::make_unique("http://www.example.com", std::chrono::seconds{10});

    // Now inject the two async requests into the client to be executed.  Moving into the client is required,
    // this passes ownership of the request to the client's background worker thread.
    client.start_request(std::move(request_with_callback_ptr), [](lift::request_ptr req_ptr, lift::response response) {
        std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
        std::cout << response << "\n\n";
    });

    // Inject the future request
    auto future = client.start_request(std::move(request_with_future_ptr));

    // Block until the future async request completes, this returns the original request and the response.
    // Note that the callback request could complete and print to stdout before or after since it is
    // running on the lift client thread and not here.
    auto [req_ptr, resp] = future.get();
    std::cout << "Lift status: " << lift::to_string(response.lift_status()) << "\n";
    std::cout << response << "\n\n"; // Will print the raw http response.

    // Block on this main thread until the lift client has completed all requests, or timed out.
    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    return 0;
}
