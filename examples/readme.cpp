#include <iostream>
#include <lift/lift.hpp>

int main()
{
    std::chrono::seconds timeout{10};

    // Synchronous requests can be created on the stack with a 10 second timeout.
    lift::request sync_request{"http://www.example.com", timeout};

    // This is the blocking synchronous HTTP call, this thread will wait until completed or timeout.
    auto sync_response = sync_request.perform();
    std::cout << "Lift status: " << lift::to_string(sync_response.lift_status()) << "\n";
    std::cout << sync_response << "\n\n"; // Will print the raw http response.

    // Creating the client starts it immediately, it spawns a background thread for executing
    // requests asynchronously and also allows for re-using http connections if possible.
    lift::client client{};

    // Asynchronous requests need to be created on the heap, this request will be fulfilled by
    // a std::future once.  Its ownership is first transferred into the lift::client while processing
    // and is transferred back upon completion or timeout.
    auto async_future_request = std::make_unique<lift::request>("http://www.example.com", timeout);

    // If you need a little more control or don't want to block on a std::future then the API allows
    // for the lift::client to invoke a lambda function upon completing or timing out a request.
    // It is important to note that the lambda will execute on the lift::client's background
    // event loop thread, avoid any heavy CPU usage in the lambda otherwise it will block other
    // outstanding requests from completing.
    auto async_callback_request = std::make_unique<lift::request>("http://www.example.com", timeout);

    // Now inject the two async requests into the client to be executed.  Moving into the client is
    // required as this passes ownership of the request to the client's background worker thread
    // while its being processed.  If you hold on to any raw pointers or references to the request
    // after transferring ownership be sure to *not* use them until the request is completed, modifying
    // a requests state during execution is prohibited.

    // Start the request that will be completed by future.
    auto future = client.start_request(std::move(async_future_request));

    // Start the request that will be completed by callback lambda.
    client.start_request(
        std::move(async_callback_request),
        [](lift::request_ptr async_callback_request_returned, lift::response async_callback_response) {
            std::cout << "Lift status: " << lift::to_string(async_callback_response.lift_status()) << "\n";
            std::cout << async_callback_response << "\n\n";
        });

    // Block until the async future request completes, this returns the original request and the response.
    // Note that the other callback request could complete and print to stdout before or after the future
    // request since it's lambda callback will be invoked on the lift::client's thread.
    auto [async_future_request_returned, async_future_response] = future.get();
    std::cout << "Lift status: " << lift::to_string(async_future_response.lift_status()) << "\n";
    std::cout << async_future_response << "\n\n"; // Will print the raw http response.

    // The lift::client destructor will block until all outstanding requests complete.

    return 0;
}
