namespace lift
{

template<typename Rep, typename Period>
auto RequestHandle::SetTimeout(
    std::chrono::duration<Rep, Period> timeout
) -> bool
{
    uint64_t timeout_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count()
    );

    if(timeout_ms > 0)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
        auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_ms));
#pragma clang diagnostic pop
        return (error_code == CURLE_OK);
    }
    return false;
}

} // lift
