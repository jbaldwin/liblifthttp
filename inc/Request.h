#pragma once

#include <curl/curl.h>
#include <uv.h>

#include <string>
#include <sstream>

class Request
{
public:
    Request();

    explicit Request(
        const std::string& url
    );
    virtual ~Request();

    Request(const Request& copy) = delete;
    Request(Request&& move) = default;
    auto operator=(const Request& assign) = delete;
    auto operator=(Request&& assign) -> Request& = default;

    auto SetUrl(const std::string& url) -> bool;
    auto GetUrl() const -> const std::string&;

    auto SetTimeoutMilliseconds(uint64_t timeout_ms) -> bool;

    auto Perform() -> bool;
    auto GetDownloadData() const -> std::string;
    auto GetTotalTimeMilliseconds() const -> uint64_t;

    auto TimedOut() -> bool;

    auto Reset() -> void;

protected:
    std::string m_url;
    CURL* m_curl_handle;
    CURLcode m_error_code;

    std::stringstream m_response_data;

    friend auto write_data(void* ptr, size_t size, size_t nmemb, void* curl_request_ptr) -> size_t;
    friend auto requests_accept_async(uv_async_t* async, int status) -> void;
};
