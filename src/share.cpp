#include "lift/share.hpp"

namespace lift
{
auto curl_share_lock(CURL*, curl_lock_data data, curl_lock_access, void* user_ptr) -> void;

auto curl_share_unlock(CURL*, curl_lock_data data, void* user_ptr) -> void;

share::share(options opts)
{
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_LOCKFUNC, curl_share_lock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_UNLOCKFUNC, curl_share_unlock);
    curl_share_setopt(m_curl_share_ptr, CURLSHOPT_USERDATA, this);

    if (opts == options::nothing)
    {
        curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_NONE);
    }
    else
    {
        if (static_cast<uint64_t>(opts) & static_cast<uint64_t>(options::dns))
        {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        }

        if (static_cast<uint64_t>(opts) & static_cast<uint64_t>(options::ssl))
        {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        }

        if (static_cast<uint64_t>(opts) & static_cast<uint64_t>(options::data))
        {
            curl_share_setopt(m_curl_share_ptr, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
        }
    }
}

share::~share()
{
    curl_share_cleanup(m_curl_share_ptr);
}

auto curl_share_lock(CURL*, curl_lock_data data, curl_lock_access, void* user_ptr) -> void
{
    auto& s = *static_cast<share*>(user_ptr);
    s.m_curl_locks[static_cast<uint64_t>(data)].lock();
}

auto curl_share_unlock(CURL*, curl_lock_data data, void* user_ptr) -> void
{
    auto& s = *static_cast<share*>(user_ptr);
    s.m_curl_locks[static_cast<uint64_t>(data)].unlock();
}

} // namespace lift
