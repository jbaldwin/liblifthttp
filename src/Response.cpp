#include "lift/Response.hpp"

namespace lift {

auto Response::GetResponseStatusCode() const -> http::StatusCode
{
    return m_status_code;
}

auto Response::GetResponseHeaders() const -> const std::vector<HeaderView>&
{
    return m_response_headers_idx;
}

auto Response::GetResponseData() const -> const std::string&
{
    return m_response_data;
}

auto Response::GetTotalTime() const -> std::chrono::milliseconds
{
    return m_total_time;
}

auto Response::GetCompletionStatus() const -> RequestStatus
{
    return m_completions_status;
}

auto Response::GetNumConnects() const -> uint64_t
{
    return m_num_connects;
}

auto Response::GetNumRedirects() const -> uint64_t
{
    return m_num_redircts;
}

} // namespace lift
