#include "EventLoop.h"

#include <zmq.hpp>

#include <iostream>
#include <string>

static auto request_add(
    CURLM* cmh,
    const std::string& url
) -> void;

int main(int argc, char* argv[])
{
    curl_global_init(CURL_GLOBAL_ALL);

    EventLoop eld{};


    curl_global_cleanup();

    return 0;
}

auto request_add(
    CURLM* cmh,
    const std::string& url
) -> void
{
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());


    curl_multi_add_handle(cmh, curl);

}


