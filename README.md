liblifthttp - The fast asynchronous C++14 HTTP client library
=============================================================

You're using curl? Do you even lift?

Copyright (c) 2017, Josh Baldwin

https://github.com/jbaldwin/liblifthttp

**liblifthttp** is a C++14 client library that provides an easy to use high throughput asynchronous HTTP request client library.  This library was designed with an easy to use client API and maximum performance for thousands of asynchronous HTTP requests on a single (or multiple) worker threads.  Additional HTTP requests can be injected into one of the worker threads with different timeouts at any point in time safely.  **TODO performance overview**

**liblifthttp** is licensed under the Apache 2.0 license.

# Overview #
* Synchronous and Asynchronous HTTP Request support.
* Easy and safe to use C++14 Client library API.

# Usage #

## Requirements
        CMake
        pthreads
        libcurl
        libuv

## Instructions

### Building
        # This will produce lib/liblifthttp.a and bin/liblifthttp_test
        mkdir Release && cd Release;
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -- -j4

## Examples

See all of the examples under the examples/ directory.

### Simple Synchronous

        lift::initialize();
        
        lift::Request request("http://www.example.com");
        request.Perform();
        std::cout << request.GetResponseData() << std::endl;
        
        lift::cleanup();

### Simple Asynchronous

See [Async Simple](https://github.com/jbaldwin/liblifthttp/blob/master/examples/async_simple.cpp)

## Performance

TODO

## Support

File bug reports, feature requests and questions using [GitHub Issues](https://github.com/jbaldwin/liblifthttp/issues)
