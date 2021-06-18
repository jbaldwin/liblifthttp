#include <iostream>
#include <lift/lift.hpp>

int main()
{
    const std::string url{"http://www.example.com"};

    // Every HTTP request in this example has a 10 second timeout to complete.
    const std::chrono::seconds timeout{10};

    // Synchronous requests can be created on the stack.
    lift::request sync_request{url, timeout};

    // Debug information about any request can be added by including a callback handler for debug
    // information.  Just pass in a lambda to capture the verbose debug information.
    sync_request.debug_info_handler(
        [](const lift::request& /*unused*/, lift::debug_info_type type, std::string_view data) {
            std::cout << "sync_request (" << lift::to_string(type) << "): " << data;
        });

    // Set the http method for this synchronous request.
    sync_request.method(lift::http::method::post);

    // Add headers to this request, they are given as a name + value pair.  Note that if the same
    // header name is specified more than once then it will appear that many times in the request.
    sync_request.header("x-lift", "lift-custom-header-data");

    // Add some data to the request body, note that if the request http verb is not post or put then
    // this data call will set the http verb to post since the request now includes a body.
    sync_request.data("lift-hello-world-data");

    // This is the blocking synchronous HTTP call, this thread will wait until the http request
    // completes or times out.
    auto sync_response = sync_request.perform();
    std::cout << "Lift status (sync): " << lift::to_string(sync_response.lift_status()) << "\n";
    std::cout << sync_response << "\n\n"; // Will print the raw http response.

    // Asynchronous requests must be created on the heap and they also need to be executed through
    // a lift::client instance.  Creating a lift::client automatically spawns a background event
    // loop thread to exceute the http requests it is given.  A lift::client also maintains a set
    // of http connections and will actively re-use available http connections when possible.
    lift::client client{};

    // Create an asynchronous request that will be fulfilled by a std::future upon its completion.
    auto async_future_request = std::make_unique<lift::request>(url, timeout);

    // Create an asynchronous request that will be fulfilled by a callback upon its completion.
    // It is important to note that the callback will be executed on the lift::client's background
    // event loop thread so it is wise to avoid any heavy CPU usage within this callback otherwise
    // other outstanding requests will be blocked from completing.
    auto async_callback_request = std::make_unique<lift::request>(url, timeout);

    // Starting the asynchronous requests requires the request ownership to be moved to the
    // lift::client while it is being processed.  Regardless of the on complete method, future or
    // callback, the original request object and its response will have their ownership moved back
    // to you upon completion.  If you hold on to any raw pointers or references to the requests
    // while they are being processed be sure not to use them until the requests complete.  Modifying
    // a request's state during execution is prohibited.

    // Start the request that will be completed by future.
    auto future = client.start_request(std::move(async_future_request));

    // Start the request that will be completed by callback.
    client.start_request(
        std::move(async_callback_request),
        [](lift::request_ptr async_callback_request_returned, lift::response async_callback_response) {
            // This on complete callback will run on the lift::client background event loop thread.
            std::cout << "Lift status (async callback): ";
            std::cout << lift::to_string(async_callback_response.lift_status()) << "\n";
            std::cout << async_callback_response << "\n\n";
        });

    // Block until the async future request completes, this returns the original request and the response.
    // Note that the other callback request could complete and print to stdout before or after the future
    // request completes since its lambda callback will be invoked on the lift::client's thread.
    auto [async_future_request_returned, async_future_response] = future.get();
    std::cout << "Lift status (async future): ";
    std::cout << lift::to_string(async_future_response.lift_status()) << "\n";
    std::cout << async_future_response << "\n\n";

    // The lift::client destructor will block until all outstanding requests complete or timeout.

    return 0;
}
