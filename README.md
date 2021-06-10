# liblifthttp - Safe Easy to use C++17 HTTP client library

[![CI](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/jbaldwin/liblifthttp/badge.svg?branch=master)](https://coveralls.io/github/jbaldwin/liblifthttp?branch=master)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/jbaldwin/liblifthttp.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/jbaldwin/liblifthttp/context:cpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/2625260f88524abfa2c2974ad9328e45)](https://www.codacy.com/gh/jbaldwin/liblifthttp/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=jbaldwin/liblifthttp&amp;utm_campaign=Badge_Grade)
[![language][badge.language]][language]
[![license][badge.license]][license]

You're using curl? Do you even lift?

**liblifthttp** is a C++17 HTTP client library that provides an easy to use API for both synchronous _and_ asynchronous requests.  It is built upon the super heavyweight champions libcurl and libuv libraries.

**liblifthttp** is licensed under the Apache 2.0 license.

## Overview
* Easy to use Synchronous and Asynchronous HTTP Request APIs.
* Safe C++17 client library API, modern memory move semantics.
* Background IO thread(s) for sending and receiving Async HTTP requests.
* Request pooling for re-using HTTP requests and sharing of connection information.

## Usage

### Examples

See all of the examples under the `examples/` directory.  Below are some simple examples
to get your started on using liblifthttp with both the synchronous and asynchronous APIs.

#### Synchronous and Asynchronous Requests
```C++
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
```

### Requirements
    C++17 compiler
        g++-9
        clang-9
    CMake
    make or ninja
    pthreads
    libcurl-devel >= 7.59
    libuv-devel
    zlib-devel
    openssl-devel (or equivalent curl support ssl library)
    stdc++fs

    Tested on:
        ubuntu:20.04
        fedora:31

### Instructions

#### Building
    # This will produce a static library to link against your project.
    mkdir Release && cd Release
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build .

#### CMake Projects

CMake options:

| Name                     | Default                       | Description                            |
|:-------------------------|:------------------------------|:---------------------------------------|
| LIFT_BUILD_EXAMPLES      | ON                            | Should the examples be built?          |
| LIFT_BUILD_TESTS         | ON                            | Should the tests be built?             |
| LIFT_CODE_COVERAGE       | OFF                           | Should code coverage be enabled?       |
| LIFT_USER_LINK_LIBRARIES | curl z uv pthread dl stdc++fs | Override lift's target link libraries. |


Note on `LIFT_USER_LINK_LIBRARIES`, if override the value then all of the default link libraries/targets must be
accounted for in the override.  E.g. if you are building with a custom curl target but defaults for everything else
then `-DLIFT_USER_LINK_LIBRARIES="custom_curl_target z uv pthread dl stdc++fs"` would be the correct setting.

##### add_subdirectory()
To use within your cmake project you can clone the project or use git submodules and then `add_subdirectory` in the parent project's `CMakeList.txt`,
assuming the lift code is in a `liblifthttp/` subdirectory of the parent project:
    add_subdirectory(liblifthttp)
To link to the `<project_name>` then use the following:
    add_executable(<project_name> main.cpp)
    target_link_libraries(<project_name> PUBLIC lifthttp)
Include lift in the project's code by simply including `#include <lift/lift.hpp>` as needed.

##### FetchContent
CMake can also include the project directly via a `FetchContent` declaration.  In your project's `CMakeLists.txt`
include the following code to download the git repository and make it available to link to.

```cmake
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

target_link_libraries(${PROJECT_NAME} PUBLIC lifthttp)
```

#### Running Tests
The tests are automatically run by GitHub Actions on all Pull Requests.  They can also be ran locally with a default
localhost instance of `nginx`.  To do so the CMake option `LIFT_LOCALHOST_TESTS=ON` must be set otherwise the tests
will use the hostname `nginx` setup in the CI settings.  After building and starting `nginx` tests can be run by issuing:

    # Invoke via cmake:
    ctest -v

    # Or invoke directly to see error messages if tests are failing:
    ./test/liblifthttp_tests

Note there are now proxy http requests that utilize an `haproxy` instance.  To run these locally you will also need
to start an instance of `haproxy`.

### Benchmarks
Using the example benchmark code and a local `nginx` instance serving its default welcome page.  All benchmarks use `keep-alive` connections.  The benchmark is compared against `wrk` as that is basically optimal performance since
`wrk` does zero parsing of the response whereas `lift` does.

Here is the CPU the benchmarks were run on:

```bash
cat /proc/cpuinfo
...
Intel(R) Core(TM) i9-9980HK CPU @ 2.40GHz
```

Here is how the benchmark application is called:

```bash
$ ./examples/lift_benchmark --help
Usage: ./examples/lift_benchmark<options> <url>
    -c --connections  HTTP Connections to use per thread.
    -t --threads      Number of threads to use.
                      evenly between each worker thread.
    -d --duration     Duration of the test in seconds
    -h --help         Print this help usage.
```

Using `nginx` as the webserver with the default `fedora` configuration.

| Connections | Threads | wrk Req/Sec | lift Req/Sec |
|------------:|--------:|------------:|-------------:|
| 1           | 1       | 31,138      | 20,785       |
| 100         | 1       | 148,121     | 44,393       |
| 100         | 2       | 220,670     | 81,335       |
| 100         | 3       | 241,839     | 104,747      |
| 100         | 4       | 275,633     | 123,481      |
| 100         | 8       | 249,845     | 143,911      |

### Support

File bug reports, feature requests and questions using [GitHub Issues](https://github.com/jbaldwin/liblifthttp/issues)

Copyright © 2017-2021, Josh Baldwin

[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-Apache--2.0-blue

[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/Apache_License
