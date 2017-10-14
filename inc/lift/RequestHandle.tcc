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
        auto error_code = curl_easy_setopt(m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(timeout_ms));
        return (error_code == CURLE_OK);
    }
    return false;
}

template<typename UserDataType>
auto RequestHandle::SetUserData(UserDataType* user_data) -> void
{
    m_user_data = static_cast<void*>(user_data);
}

template<typename UserDataType>
auto RequestHandle::GetUserData() -> UserDataType*
{
    return static_cast<UserDataType*>(m_user_data);
}

} // lift
