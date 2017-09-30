#include "lift/AsyncRequest.h"

namespace lift
{

AsyncRequest::AsyncRequest()
    :
        Request()
{

}

AsyncRequest::AsyncRequest(
    const std::string& url,
    uint64_t timeout_ms
)
    :
        Request(url, timeout_ms)
{

}

auto AsyncRequest::setRequestStatus(
    CURLcode curl_request_code
) -> void
{
    m_status_code = Request::curl_code2request_status(curl_request_code);
}

} // lift
