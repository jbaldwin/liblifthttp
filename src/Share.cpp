#include "lift/Share.hpp"

namespace lift {

auto curl_share_lock(
    CURL*,
    curl_lock_data data,
    curl_lock_access,
    void* user_ptr) -> void;

auto curl_share_unlock(
    CURL*,
    curl_lock_data data,
    void* user_ptr) -> void;

Share::Share(
    ShareOptions share_options)
{
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_LOCKFUNC, curl_share_lock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_UNLOCKFUNC, curl_share_unlock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_USERDATA, this);

    if (share_options == ShareOptions::NOTHING) {
        curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_NONE);
    } else {
        if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::DNS)) {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        }

        if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::SSL)) {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        }

        if (static_cast<uint64_t>(share_options) & static_cast<uint64_t>(ShareOptions::DATA)) {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
        }
    }
}

Share::~Share()
{
    curl_share_cleanup(m_curl_share_ptr);
}

auto curl_share_lock(
    CURL*,
    curl_lock_data data,
    curl_lock_access,
    void* user_ptr) -> void
{
    auto& share = *static_cast<Share*>(user_ptr);
    share.m_curl_locks[static_cast<uint64_t>(data)].lock();
}

auto curl_share_unlock(
    CURL*,
    curl_lock_data data,
    void* user_ptr) -> void
{
    auto& share = *static_cast<Share*>(user_ptr);
    share.m_curl_locks[static_cast<uint64_t>(data)].unlock();
}

} // namespace lift
