libcfh - The asynchronous fast C++14 HTTP client library
========================================================

Copyright (c) 2017, Josh Baldwin

https://github.com/jbaldwin/libcfh

**libcfh** is a C++14 client library that provides an easy to use high throughput asynchronous HTTP request client library.  This library was designed with an easy to use client API and maximum performance for thousands of asynchronous HTTP requests  on a single (or multiple) worker threads.  Additional HTTP requests can be injected into one of the worker threads with different timeouts at any point in time safely.  **TODO performance overview**

**libcfh** is licensed under the Apache 2.0 license.

# Overview #
* Synchronous HTTP Request support.
* Asynchronous HTTP Request support.
* C++14 Client library API.

# Usage #

## Requirements
        CMake
        pthreads
        libcurl
        libuv

## Instructions

### Building
        # This will produce lib/libcfh.a and bin/libcfh_test
        mkdir Release && cd Release;
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . -- -j4

## Example Synchronous
        // A simple synchronous request
        curl_global_init(CURL_GLOBAL_ALL);
        
        Request request("http://www.example.com");
        request.Perform();
        std::cout << request.GetDownloadData() << std::endl;
        
        curl_global_cleanup();

## Example Asynchronous

See example/async_simple.cpp

## Performance

TODO

## Support

File bug reports, feature requests and questions using [GitHub Issues](https://github.com/jbaldwin/libcfh/issues)
