# liblifthttp - Safe Easy to use C++17 HTTP client library

[![CI](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)](https://github.com/jbaldwin/liblifthttp/workflows/build/badge.svg)
[![Coverage Status](https://coveralls.io/repos/github/jbaldwin/liblifthttp/badge.svg?branch=master)](https://coveralls.io/github/jbaldwin/liblifthttp?branch=master)
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

## Known Bugs/Issues
*   libcurl 7.81.0 is unsupported due to a known libcurl bug in the multi handle code.  Unfortunately ubuntu 22.04 comes with this version installed by default, you will need to manually install a different version of libcurl or build libcurl from source and link to it to avoid segfaults in asynchronous http requests via liblift.  See [here](https://github.com/jbaldwin/liblifthttp/issues/142) for more information

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
    const std::string url{"http://www.example.com"};

    // Every HTTP request in this example has a 10 second timeout to complete.
    const std::chrono::seconds timeout{10};

    // Synchronous requests can be created on the stack.
    lift::request sync_request{url, timeout};

    // Debug information about any request can be added by including a callback handler for debug
    // information.  Just pass in a lambda to capture the verbose debug information.
    sync_request.debug_info_handler(
        [](const lift::request& /*unused*/, lift::debug_info_type type, std::string_view data)
        { std::cout << "sync_request (" << lift::to_string(type) << "): " << data; });

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
        [](lift::request_ptr async_callback_request_returned, lift::response async_callback_response)
        {
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
```

### Requirements
```bash
C++17 compilers tested
    g++ [9, 10, 11, 12, 13, 14]
    clang [9, 14, 15, 16, 17, 18, 19]
CMake
make or ninja
pthreads
libcurl-devel >= 7.59
    *UNSUPPORTED* 7.81.0 has a known libcurl multi bug that was fixed in 7.82.0.
    ubuntu-22.04 apt pacakges have this broken version, see ci-ubuntu.yml on how to
    manually build and link to a working version of curl.
libuv-devel
zlib-devel
openssl-devel (or equivalent curl support ssl library)
stdc++fs

Tested on:
    ubuntu:20.04
    ubuntu:22.04 (with custom libcurl built)
    ubuntu:24.04
    fedora:41
```

### Instructions

#### Building
```bash
# This will produce a static library to link against your project.
mkdir Release && cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

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
then `-DLIFT_USER_LINK_LIBRARIES="custom_curl_target;z;uv;pthread;dl;stdc++fs"` would be the correct setting.

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
localhost instance of `nginx` and `haproxy`.  To do so the CMake option `LIFT_LOCALHOST_TESTS=ON` must be set otherwise the tests
will use the hostname `nginx` setup in the CI settings.  After building and starting `nginx` and `haproxy` tests can be run by issuing:

```bash
# Invoke via cmake:
ctest -v

# Or invoke directly to see error messages if tests are failing:
./test/liblifthttp_tests
```

Note:
* `nginx` should be default install/configuration running on port `80`.
* `haproxy` should be running on port `*3128` with a backend pointing at the `nginx` instance. See `docker/build/haproxy/haproxy.cfg` to update the local configuration.


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

Copyright © 2017-2024, Josh Baldwin

[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-Apache--2.0-blue

[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/Apache_License
