liblifthttp - Safe Easy to use C++17 HTTP client library.
=========================================================

[![CI](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/jbaldwin/liblifthttp/badge.svg?branch=master)](https://coveralls.io/github/jbaldwin/liblifthttp?branch=master)
[![language][badge.language]][language]
[![license][badge.license]][license]

[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-Apache--2.0-blue

[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/Apache_License

You're using curl? Do you even lift?

https://github.com/jbaldwin/liblifthttp

**liblifthttp** is a C++17 HTTP client library that provides an easy to use API for both synchronous _and_ asynchronous requests.  It is built upon the rock solid libcurl and libuv libraries.

**liblifthttp** is licensed under the Apache 2.0 license.

# Overview #
* Easy to use Synchronous and Asynchronous HTTP Request APIs.
* Safe C++17 client library API, modern memory move semantics.
* Background IO thread(s) for sending and receiving Async HTTP requests.
* Request pooling for re-using HTTP requests.

# Usage #

## Examples

See all of the examples under the examples/ directory.  Below are some simple examples
to get your started on using liblifthttp with both the synchronous and asynchronous APIs.

```C++
// libcurl requires some global functions to be called before being used.
// LibLiftHttp will call these appropriately if you place the following in
// the projects main.cpp file(s) where necessary.
static lift::GlobalScopeInitializer g_lifthttp_gsi{};
```

### Simple Synchronous
```C++
// Synchronous requests can be created on the stack.
lift::Request request{"http://www.example.com"};
// This is the blocking synchronous HTTP call.
auto response = request.Perform();
std::cout << "LiftStatus: " << lift::to_string(response.LiftStatus()) << "\n";
std::cout << "HTTP Status Code: " << lift::http::to_string(response.StatusCode()) << "\n";
for(const auto& header : response.Headers())
{
    std::cout << header.Name() << ": " << header.Value() << "\n";
}
std::cout << response.Data();
```

### Simple Asynchronous
```C++
// Creating the event loop starts it immediately, it spawns a background thread for executing requests.
lift::EventLoop loop{};

// Create the request just like we did in the sync version, but now provide a lambda for on completion.
// NOTE: that the Lambda is executed ON the Lift event loop background thread.  If you want to handle
// on completion processing on this main thread you need to std::move() it back via a queue or inter-thread
// communication.  This is imporant if any resources are shared between the threads.
// NOTE: The request is created on the heap so ownership can be passed easily via an std::unique_ptr
// to the lift::EventLoop!  lift::Request::make() is a handy function to easily do so.
auto request_ptr = lift::Request::make(
    "http://www.example.com",
    std::chrono::seconds{10}, // Give the request 10 seconds to complete or timeout.
    [](lift::RequestPtr req_ptr, lift::Response response)
    {
        std::cout << "LiftStatus: " << lift::to_string(response.LiftStatus()) << "\n";
        std::cout << "HTTP Status Code: " << lift::http::to_string(response.StatusCode()) << "\n";
        for(const auto& header : response.Headers())
        {
            std::cout << header.Name() << ": " << header.Value() << "\n";
        }
        std::cout << response.Data();
    }
);

// Now inject the request into the event to be executed.  Moving into the event loop is required,
// this passes ownership of the request to the event loop background worker thread.
loop.StartRequest(std::move(request_ptr));

// Block on this main thread until the lift event loop background thread has completed the request, or timed out.
while(loop.ActiveRequestCount() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
}

// When loop goes out of scope here it will automatically stop the background thread and cleanup all resources.
```

## Requirements
    C++17 compiler
        g++-9
        clang-9
    CMake
    make or ninja
    std::thread
    libcurl-devel
    libuv-devel
    zlib-devel
    stdc++fs

    Tested on:
        ubuntu:18.04
        fedora:31

## Instructions

### Building
    # This will produce a static library to link against your project.
    mkdir Release && cd Release
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .

### CMake Projects

CMake options:

    LIFT_BUILD_EXAMPLES "Build the examples. Default=ON"
    LIFT_BUILD_TESTS    "Build the tests. Default=ON"
    LIFT_CODE_COVERAGE  "Enable code coverage, tests must also be enabled. Default=OFF"

NOTE: by default liblifthttp will attempt to use system versions of `libcurl-devel`, `libuv-devel`, `libcrypto-devel`, `libssl-devel`, and `libcares-devel`.  If your project(s) require a custom built version
of `libcurl` or any of the other libraries that curl links to then you can specify the following `cmake` variables to override where liblifthttp
will link `libcurl-devel` development libraries.  These can be dynamic or static libraries.  Note that a custom `libuv-devel` link is not currently supported.

    ${LIFT_CURL_INCLUDE} # The curl.h header location, default is empty.
    ${LIFT_LIBSSL}       # The ssl library to link against, default is empty.
    ${LIFT_LIBCRYPTO}    # The crypto library to link against, default is empty.
    ${LIFT_LIBCURL}      # The curl library to link against, default is '-lcurl'.
    ${LIFT_LIBCARES}     # The c-ares (dns) library to link against, default is empty.

#### add_subdirectory()
To use within your cmake project you can clone the project or use git submodules and then `add_subdirectory` in the parent project's `CMakeList.txt`,
assuming the lift code is in a `liblifthttp/` subdirectory of the parent project:

    add_subdirectory(liblifthttp)

To link to the `<project_name>` then use the following:

    add_executable(<project_name> main.cpp)
    target_link_libraries(<project_name> PRIVATE lifthttp)

Include lift in the project's code by simply including `#include <lift/lift.hpp>` as needed.

#### FetchContent
CMake can also include the project directly via a `FetchContent` declaration.  In your project's `CMakeLists.txt`
include the following code to download the git repository and make it available to link to.

    cmake_minimum_required(VERSION 3.11)

    # ... cmake project stuff ...

    include(FetchContent)
    FetchContent_Declare(
        lifthttp
        GIT_REPOSITORY https://github.com/jbaldwin/liblifthttp.git
        GIT_TAG        <TAG_OR_GIT_HASH>
    )
    FetchContent_MakeAvailable(lifthttp)

    # ... cmake project more stuff ...

    target_link_libraries(${PROJECT_NAME} PRIVATE lifthttp)

## Benchmarks
Using the example benchmark code and a local `nginx` instance serving its default welcome page.  All benchmarks use `keep-alive` connections.  The benchmark is compared against `wrk` as that is basically optimal performance since
`wrk` does zero parsing of the response whereas `lift` does.

Here is the CPU the benchmarks were run on:

    cat /proc/cpuinfo
    ...
    Intel(R) Core(TM) i7-9750H CPU @ 2.60GHz

Here is how the benchmark application is called (similiar to `wrk`):

    $ ./examples/lift_benchmark
    ./examples/lift_benchmark <url> <duration_seconds> <connections> <threads>

Using `nginx` as the webserver with the default `ubuntu` configuration.

| Connections | Threads | wrk Req/Sec | lift Req/Sec |
|------------:|--------:|------------:|-------------:|
| 1           | 1       | 28,093      | 18,009       |
| 100         | 1       | 130,946     | 47,077       |
| 100         | 2       | 165,981     | 73,181       |
| 100         | 3       | 173,978     | 88,881       |
| 100         | 4       | 171,778     | 96,355       |

## Support

File bug reports, feature requests and questions using [GitHub Issues](https://github.com/jbaldwin/liblifthttp/issues)

Copyright © 2017-2020, Josh Baldwin
