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

## Known Bugs/Issues
*   libcurl 7.81.0 is unsupported due to a known libcurl bug in the multi handle code.  Unfortunately ubuntu 22.04 comes with this version installed by default, you will need to manually install a different version of libcurl or build libcurl from source and link to it to avoid segfaults in asynchronous http requests via liblift.  See [here](https://github.com/jbaldwin/liblifthttp/issues/142) for more information
*   libcurl share does not work across multiple threads, expect segfaults if you try and use the `lift::share` objects across threads.  The bug is apparently very difficult to fix and is unlikely to be fixed anytime soon after talking with the libcurl maintainer.

## Usage

### Examples

See all of the examples under the `examples/` directory.  Below are some simple examples
to get your started on using liblifthttp with both the synchronous and asynchronous APIs.

#### Synchronous and Asynchronous Requests
```C++
${EXAMPLE_README_CPP}
```

### Requirements
```bash
C++17 compilers tested
    g++-9
    g++-11
    clang-9
    clang-14
CMake
make or ninja
pthreads
libcurl-devel >= 7.59
    *UNSUPPORTED* 7.81.0 has a known libcurl mutli* bug that was fixed in 7.82.0.
libuv-devel
zlib-devel
openssl-devel (or equivalent curl support ssl library)
stdc++fs

Tested on:
    ubuntu:20.04
    ubuntu:22.04 (with custom libcurl built)
    fedora:31
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
localhost instance of `nginx`.  To do so the CMake option `LIFT_LOCALHOST_TESTS=ON` must be set otherwise the tests
will use the hostname `nginx` setup in the CI settings.  After building and starting `nginx` tests can be run by issuing:

```bash
# Invoke via cmake:
ctest -v

# Or invoke directly to see error messages if tests are failing:
./test/liblifthttp_tests
```

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

Copyright © 2017-2022, Josh Baldwin

[badge.language]: https://img.shields.io/badge/language-C%2B%2B17-yellow.svg
[badge.license]: https://img.shields.io/badge/license-Apache--2.0-blue

[language]: https://en.wikipedia.org/wiki/C%2B%2B17
[license]: https://en.wikipedia.org/wiki/Apache_License