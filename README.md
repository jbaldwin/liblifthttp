liblifthttp - The fast asynchronous C++17 HTTP client library
=============================================================

You're using curl? Do you even lift?

Copyright (c) 2017, Josh Baldwin

https://github.com/jbaldwin/liblifthttp

**liblifthttp** is a C++17 client library that provides an easy to use high throughput asynchronous HTTP request client library.  This library was designed with an easy to use client API and maximum performance for thousands of asynchronous HTTP requests on a single (or multiple) worker threads.  Additional HTTP requests can be injected into one of the worker threads with different timeouts at any point in time safely.  The asynchronuos API can perform upwards of 30,000 HTTP requests / second on a single 2.8GHZ core.

**liblifthttp** is licensed under the Apache 2.0 license.

# Overview #
* Synchronous and Asynchronous HTTP Request support.
* Easy and safe to use C++17 Client library API.
* Background IO thread for sending and receiving HTTP requests.
* Request pooling for re-using HTTP requests.

# Usage #

## Requirements
    C++17 compiler (g++/clang++)
    CMake
    pthreads/std::thread
    libcurl
    libuv

## Instructions

### Building
    # This will produce lib/liblifthttp.a and bin/liblifthttp_test + all the examples
    mkdir Release && cd Release
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . -- -j4 # change 4 to number of cores available

## Examples

See all of the examples under the examples/ directory.

### Simple Synchronous
```C++
// Initialize must be called first before using the LiftHttp library.
lift::GlobalScopeInitializer lift_init{};
lift::RequestPool pool{};
auto request = pool.Produce("http://www.example.com");

request->Perform();  // This call is the blocking synchronous HTTP call.

std::cout << request->GetResponseData() << "\n";
```

### Simple Asynchronous
```C++
// Event loops in Lift come with their own RequestPool, no need to provide one.
// Creating the event loop starts it immediately, it spawns a background thread for executing requests.
lift::EventLoop loop{};
auto& pool = loop.GetRequestPool();

// Create the request just like we did in the sync version, but we provide a lambda for on completion.
// Note: that the Lambda is executed ON the Lift event loop thread.  If you want to handle on completion
// processing on this main thread you need to std::move it back via a queue or inter-thread communication.
auto request = pool.Produce(
    "http://www.example.com",
    [](lift::RequestHandle r) { std::cout << r->GetResponseData(); }, // on destruct 'r' will return to the pool.
    10s, // optional time out parameter here
);

// Now inject the request into the event to be executed.  Moving into the event loop is required,
// this passes ownership of the request to the event loop.
loop.StartRequest(std::move(request));

// Block on this main thread until the lift event loop thread has completed the request, or timed out.
while(loop.GetActiveRequestCount() > 0) {
    std::this_thread::sleep_for(10ms);
}

// When loop goes out of scope here it will automatically stop the background thread.
```

## Support

File bug reports, feature requests and questions using [GitHub Issues](https://github.com/jbaldwin/liblifthttp/issues)
