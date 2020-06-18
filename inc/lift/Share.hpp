#pragma once

#include <array>
#include <memory>
#include <mutex>

#include <curl/curl.h>

namespace lift {

class Executor;

enum class ShareOptions : uint64_t {
    /// Share nothing across requests.
    NOTHING = 0,
    /// Share DNS information across requests.
    DNS = 1 << 1,
    /// Share SSL information across requests.
    SSL = 1 << 2,
    /// Share Data pipeline'ing across requests.
    DATA = 1 << 3,

    /// Share DNS with SSL.
    DNS_SSL = (DNS + SSL),
    /// Share DNS with Data.
    DNS_DATA = (DNS + DATA),
    /// Share SSL with Data.
    SSL_DATA = (SSL + DATA),
    /// Share all available types.
    ALL = (DNS + SSL + DATA)
};

class Share {
    friend Executor;

public:
    /**
     * @param share_options The specific items to share between requests.
     */
    Share(
        ShareOptions share_options);
    ~Share();

    Share(const Share&) = delete;
    Share(Share&&) = delete;
    auto operator=(const Share&) noexcept -> Share& = delete;
    auto operator=(Share&&) noexcept -> Share& = delete;

private:
    CURLSH* m_curl_share_ptr { curl_share_init() };

    std::array<std::mutex, static_cast<uint64_t>(CURL_LOCK_DATA_LAST)> m_curl_locks {};

    friend auto curl_share_lock(
        CURL* curl_ptr,
        curl_lock_data data,
        curl_lock_access access,
        void* user_ptr) -> void;

    friend auto curl_share_unlock(
        CURL* curl_ptr,
        curl_lock_data data,
        void* user_ptr) -> void;
};

using SharePtr = std::shared_ptr<Share>;

} // namespace lift
