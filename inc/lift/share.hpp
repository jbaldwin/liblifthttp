#pragma once

#include <array>
#include <memory>
#include <mutex>

#include <curl/curl.h>

namespace lift
{
class executor;

class share : public std::enable_shared_from_this<share>
{
    friend executor;

public:
    enum class options : uint64_t
    {
        /// Share nothing across requests.
        nothing = 0,
        /// Share DNS information across requests.
        dns = 1 << 1,
        /// Share SSL information across requests.
        ssl = 1 << 2,
        /// Share Data pipeline'ing across requests.
        data = 1 << 3,

        /// Share DNS with SSL.
        dns_ssl = (dns + ssl),
        /// Share DNS with Data.
        dns_data = (dns + data),
        /// Share SSL with Data.
        ssl_data = (ssl + data),
        /// Share all available types.
        all = (dns + ssl + data)
    };

    /**
     * @param opts The specific items to share between requests.
     */
    explicit share(options opts);
    ~share();

    share(const share&) = delete;
    share(share&&)      = delete;
    auto operator=(const share&) noexcept -> share& = delete;
    auto operator=(share&&) noexcept -> share& = delete;

    static auto make_shared(options opts) -> std::shared_ptr<share> { return std::make_shared<share>(std::move(opts)); }

private:
    CURLSH* m_curl_share_ptr{curl_share_init()};

    std::array<std::recursive_mutex, static_cast<uint64_t>(CURL_LOCK_DATA_LAST)> m_curl_locks{};

    friend auto curl_share_lock(CURL* curl_ptr, curl_lock_data data, curl_lock_access access, void* user_ptr) -> void;

    friend auto curl_share_unlock(CURL* curl_ptr, curl_lock_data data, void* user_ptr) -> void;
};

using share_ptr = std::shared_ptr<share>;

} // namespace lift
