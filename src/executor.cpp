#include "lift/executor.hpp"
#include "lift/client.hpp"
#include "lift/init.hpp"

namespace lift
{
auto curl_write_header(char* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t;

auto curl_write_data(void* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t;

auto curl_xfer_info(
    void*      clientp,
    curl_off_t download_total_bytes,
    curl_off_t download_now_bytes,
    curl_off_t upload_total_bytes,
    curl_off_t upload_now_bytes) -> int;

executor::executor(request* request, share* share) : m_request_sync(request), m_request(m_request_sync), m_response()
{
    if (share != nullptr)
    {
        m_curl_share_handle = share->m_curl_share_ptr;
    }
}

executor::executor(client* c) : m_client(c)
{
}

executor::~executor()
{
    reset();
    curl_easy_cleanup(m_curl_handle);
}

auto executor::start_async(request_ptr req_ptr, share* share) -> void
{
    m_request_async = std::move(req_ptr);
    m_request       = m_request_async.get();
    if (share != nullptr)
    {
        m_curl_share_handle = share->m_curl_share_ptr;
    }
}

auto executor::perform() -> response
{
    global_init();

    prepare();

    auto curl_error_code     = curl_easy_perform(m_curl_handle);
    m_response.m_lift_status = convert(curl_error_code);
    copy_curl_to_response();

    global_cleanup();

    return std::move(m_response);
}

auto executor::prepare() -> void
{
    curl_easy_setopt(m_curl_handle, CURLOPT_PRIVATE, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, curl_write_header);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOSIGNAL, 1L);

    curl_easy_setopt(m_curl_handle, CURLOPT_URL, m_request->url().c_str());

    switch (m_request->method())
    {
        case http::method::unknown: // default to GET on unknown/bad value.
            /* INTENTIONAL FALLTHROUGH */
        case http::method::get:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTPGET, 1L);
            break;
        case http::method::head:
            curl_easy_setopt(m_curl_handle, CURLOPT_NOBODY, 1L);
            break;
        case http::method::post:
            curl_easy_setopt(m_curl_handle, CURLOPT_POST, 1L);
            break;
        case http::method::put:
            curl_easy_setopt(m_curl_handle, CURLOPT_PUT, 1L);
            break;
        case http::method::delete_t:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        case http::method::connect:
            curl_easy_setopt(m_curl_handle, CURLOPT_CONNECT_ONLY, 1L);
            break;
        case http::method::options:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
            break;
        case http::method::patch:
            curl_easy_setopt(m_curl_handle, CURLOPT_CUSTOMREQUEST, "PATCH");
            break;
    }

    switch (m_request->version())
    {
        case http::version::unknown: // default to USE_BEST on unknown/bad value.
            /* INTENTIONAL FALLTHROUGH */
        case http::version::use_best:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
            break;
        case http::version::v1_0:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            break;
        case http::version::v1_1:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
            break;
        case http::version::v2_0:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
            break;
        case http::version::v2_0_tls:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
            break;
        case http::version::v2_0_only:
            curl_easy_setopt(m_curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
            break;
    }

    // Synchronous requests get their timeout value set directly on the curl easy handle.
    // Asynchronous requests will handle timeouts on the event loop due to Connection Time.
    if (m_request_sync != nullptr)
    {
        if (m_request->connect_timeout().has_value())
        {
            curl_easy_setopt(
                m_curl_handle,
                CURLOPT_CONNECTTIMEOUT_MS,
                static_cast<long>(m_request->connect_timeout().value().count()));
        }

        if (m_request->timeout().has_value())
        {
            curl_easy_setopt(
                m_curl_handle, CURLOPT_TIMEOUT_MS, static_cast<long>(m_request->timeout().value().count()));
        }
    }

    // Connection timeout is handled when injecting into the CURLM* event loop for asynchronous requests.

    if (m_request->follow_redirects())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(m_curl_handle, CURLOPT_MAXREDIRS, static_cast<long>(m_request->max_redirects()));
    }
    else
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 0L);
    }

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYPEER, (m_request->verify_ssl_peer()) ? 1L : 0L);
    // Note that 1L is valid, but curl docs say its basically deprecated.
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYHOST.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYHOST, (m_request->verify_ssl_host()) ? 2L : 0L);
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYSTATUS.html
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYSTATUS, (m_request->verify_ssl_status()) ? 1L : 0L);

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERT.html
    if (const auto& cert = m_request->ssl_cert(); cert.has_value())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLCERT, cert.value().c_str());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERTTYPE.html
    if (const auto& cert_type = m_request->ssl_cert_type(); cert_type.has_value())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLCERTTYPE, to_string(cert_type.value()).data());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLKEY.html
    if (const auto& key = m_request->ssl_key(); key.has_value())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_SSLKEY, key.value().c_str());
    }
    // https://curl.haxx.se/libcurl/c/CURLOPT_KEYPASSWD.html
    if (const auto& password = m_request->key_password(); password.has_value())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_KEYPASSWD, password.value().data());
    }

    // Set proxy information for the requst if provided.
    // https://curl.haxx.se/libcurl/c/CURLOPT_PROXY.html
    if (m_request->proxy().has_value())
    {
        auto& proxy_data = m_request->proxy().value();

        curl_easy_setopt(m_curl_handle, CURLOPT_PROXY, proxy_data.m_host.data());

        switch (proxy_data.m_type)
        {
            case proxy_type::https:
                curl_easy_setopt(m_curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
                break;
            case proxy_type::http:
                /* intentional fallthrough */
            default:
                curl_easy_setopt(m_curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
                break;
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_PROXYPORT, proxy_data.m_port);

        if (proxy_data.m_username.has_value())
        {
            curl_easy_setopt(m_curl_handle, CURLOPT_PROXYUSERNAME, proxy_data.m_username.value().data());
        }
        if (proxy_data.m_password.has_value())
        {
            curl_easy_setopt(m_curl_handle, CURLOPT_PROXYPASSWORD, proxy_data.m_password.value().data());
        }

        if (proxy_data.m_auth_types.has_value())
        {
            int64_t auth_types{0};
            for (const auto& auth_type : proxy_data.m_auth_types.value())
            {
                switch (auth_type)
                {
                    case http_auth_type::basic:
                        auth_types |= CURLAUTH_BASIC;
                        break;
                    case http_auth_type::any:
                        auth_types |= CURLAUTH_ANY;
                        break;
                    case http_auth_type::any_safe:
                        auth_types |= CURLAUTH_ANYSAFE;
                        break;
                }
            }

            if (auth_types != 0)
            {
                curl_easy_setopt(m_curl_handle, CURLOPT_PROXYAUTH, auth_types);
            }
        }
    }

    const auto& encodings = m_request->accept_encodings();
    if (encodings.has_value())
    {
        if (!encodings.value().empty())
        {
            std::size_t length{0};
            for (const auto& e : encodings.value())
            {
                length += e.length() + 2; // for ", "
            }

            std::string joined{};
            joined.reserve(length);

            bool first{true};
            for (auto& e : encodings.value())
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    joined.append(", ");
                }
                joined.append(e);
            }

            // strings are copied into libcurl except for POSTFIELDS.
            curl_easy_setopt(m_curl_handle, CURLOPT_ACCEPT_ENCODING, joined.c_str());
        }
        else
        {
            // From the CURL docs (https://curl.haxx.se/libcurl/c/CURLOPT_ACCEPT_ENCODING.html):
            // 'To aid applications not having to bother about what specific algorithms this particular
            // libcurl build supports, libcurl allows a zero-length string to be set ("") to ask for an
            // Accept-Encoding: header to be used that contains all built-in supported encodings.'
            curl_easy_setopt(m_curl_handle, CURLOPT_ACCEPT_ENCODING, "");
        }
    }

    // Headers
    if (m_curl_request_headers != nullptr)
    {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }

    for (auto& header : m_request->m_request_headers)
    {
        m_curl_request_headers = curl_slist_append(m_curl_request_headers, header.data().data());
    }

    if (m_curl_request_headers != nullptr)
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, m_curl_request_headers);
    }
    else
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_HTTPHEADER, nullptr);
    }

    // DNS resolve hosts
    if (!m_request->m_resolve_hosts.empty() || (m_client != nullptr && !m_client->m_resolve_hosts.empty()))
    {
        if (m_curl_resolve_hosts != nullptr)
        {
            curl_slist_free_all(m_curl_resolve_hosts);
            m_curl_resolve_hosts = nullptr;
        }

        for (const auto& resolve_host : m_request->m_resolve_hosts)
        {
            m_curl_resolve_hosts =
                curl_slist_append(m_curl_resolve_hosts, resolve_host.curl_formatted_resolve_host().data());
        }

        if (m_client != nullptr)
        {
            for (const auto& resolve_host : m_client->m_resolve_hosts)
            {
                m_curl_resolve_hosts =
                    curl_slist_append(m_curl_resolve_hosts, resolve_host.curl_formatted_resolve_host().data());
            }
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_RESOLVE, m_curl_resolve_hosts);
    }

    // POST or MIME data
    if (m_request->m_request_data_set)
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(m_request->data().size()));
        curl_easy_setopt(m_curl_handle, CURLOPT_POSTFIELDS, m_request->data().data());
    }
    else if (m_request->m_mime_fields_set)
    {
        m_mime_handle = curl_mime_init(m_curl_handle);

        for (const auto& mime_field : m_request->mime_fields())
        {
            auto* field = curl_mime_addpart(m_mime_handle);

            if (std::holds_alternative<std::string>(mime_field.value()))
            {
                curl_mime_name(field, mime_field.name().data());
                const auto& value = std::get<std::string>(mime_field.value());
                curl_mime_data(field, value.data(), value.length());
            }
            else
            {
                curl_mime_filename(field, mime_field.name().data());
                curl_mime_filedata(field, std::get<std::filesystem::path>(mime_field.value()).c_str());
            }
        }

        curl_easy_setopt(m_curl_handle, CURLOPT_MIMEPOST, m_mime_handle);
    }

    if (m_request->m_on_transfer_progress_handler != nullptr)
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFOFUNCTION, curl_xfer_info);
        curl_easy_setopt(m_curl_handle, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 0L);
    }
    else
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_NOPROGRESS, 1L);
    }

    if (const auto& timeout = m_request->happy_eyeballs_timeout(); timeout.has_value())
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS, static_cast<long>(timeout.value().count()));
    }

    // Note that this will lock the mutexes in the share callbacks.
    if (m_curl_share_handle != nullptr)
    {
        curl_easy_setopt(m_curl_handle, CURLOPT_SHARE, m_curl_share_handle);
    }
}

auto executor::copy_curl_to_response() -> void
{
    long http_response_code = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &http_response_code);
    m_response.m_status_code = http::to_enum(static_cast<uint16_t>(http_response_code));

    long http_version = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_HTTP_VERSION, &http_version);
    m_response.m_version = static_cast<http::version>(http_version);

    double total_time = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_TOTAL_TIME, &total_time);
    // std::duration defaults to seconds, so don't need to duration_cast total time to seconds.
    m_response.m_total_time = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>{total_time}).count());

    long connect_count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_NUM_CONNECTS, &connect_count);
    m_response.m_num_connects = (connect_count >= std::numeric_limits<uint8_t>::max())
                                    ? std::numeric_limits<uint8_t>::max()
                                    : static_cast<uint8_t>(connect_count);

    long redirect_count = 0;
    curl_easy_getinfo(m_curl_handle, CURLINFO_REDIRECT_COUNT, &redirect_count);
    m_response.m_num_redirects = (redirect_count >= std::numeric_limits<uint8_t>::max())
                                     ? std::numeric_limits<uint8_t>::max()
                                     : static_cast<uint8_t>(redirect_count);
}

auto executor::set_timesup_response(std::chrono::milliseconds total_time) -> void
{
    m_response.m_status_code   = lift::http::status_code::http_504_gateway_timeout;
    m_response.m_total_time    = static_cast<uint32_t>(total_time.count());
    m_response.m_num_connects  = 0;
    m_response.m_num_redirects = 0;
}

auto executor::reset() -> void
{
    if (m_mime_handle != nullptr)
    {
        curl_mime_free(m_mime_handle);
        m_mime_handle = nullptr;
    }

    if (m_curl_request_headers != nullptr)
    {
        curl_slist_free_all(m_curl_request_headers);
        m_curl_request_headers = nullptr;
    }

    if (m_curl_resolve_hosts != nullptr)
    {
        curl_slist_free_all(m_curl_resolve_hosts);
        m_curl_resolve_hosts = nullptr;
    }

    // Regardless of sync/async all three pointers get reset to nullptr.
    m_request_sync  = nullptr;
    m_request_async = nullptr;
    m_request       = nullptr;

    m_timeout_iterator.reset();
    m_on_complete_handler_processed = false;
    m_response                      = response{};

    curl_easy_setopt(m_curl_handle, CURLOPT_SHARE, nullptr);
    m_curl_share_handle = nullptr;

    curl_easy_reset(m_curl_handle);
}

auto executor::convert(CURLcode curl_code) -> lift_status
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (curl_code)
    {
        case CURLcode::CURLE_OK:
            return lift_status::success;
        case CURLcode::CURLE_GOT_NOTHING:
            return lift_status::response_empty;
        case CURLcode::CURLE_OPERATION_TIMEDOUT:
            return lift_status::timeout;
        case CURLcode::CURLE_COULDNT_CONNECT:
            return lift_status::connect_error;
        case CURLcode::CURLE_COULDNT_RESOLVE_HOST:
            return lift_status::connect_dns_error;
        case CURLcode::CURLE_SSL_CONNECT_ERROR:
            return lift_status::connect_ssl_error;
        case CURLcode::CURLE_WRITE_ERROR:
            return lift_status::download_error;
        case CURLcode::CURLE_SEND_ERROR:
            return lift_status::error_failed_to_start;
        default:
            return lift_status::error;
    }
#pragma GCC diagnostic pop
}

auto curl_write_header(char* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t
{
    auto*        executor_ptr = static_cast<executor*>(user_ptr);
    auto&        response     = executor_ptr->m_response;
    const size_t data_length  = size * nitems;

    std::string_view data_view{buffer, data_length};

    if (data_view.empty())
    {
        return data_length;
    }

    // Ignore empty header lines from curl.
    if (data_length == 2 && data_view == "\r\n")
    {
        return data_length;
    }
    // Ignore the HTTP/ 'header' line from curl.
    constexpr size_t HTTPSLASH_LEN = 5;
    if (data_length >= 4 && data_view.substr(0, HTTPSLASH_LEN) == "HTTP/")
    {
        return data_length;
    }

    // Drop the trailing \r\n from the header.
    if (data_length >= 2)
    {
        size_t rm_size = (data_view[data_length - 1] == '\n' && data_view[data_length - 2] == '\r') ? 2 : 0;
        data_view.remove_suffix(rm_size);
    }

    response.m_headers.emplace_back(std::string{data_view.data(), data_view.length()});

    return data_length; // return original size for curl to continue processing
}

auto curl_write_data(void* buffer, size_t size, size_t nitems, void* user_ptr) -> size_t
{
    auto*  executor_ptr = static_cast<executor*>(user_ptr);
    auto&  response     = executor_ptr->m_response;
    size_t data_length  = size * nitems;

    std::copy(
        static_cast<const char*>(buffer),
        static_cast<const char*>(buffer) + data_length,
        std::back_inserter(response.m_data));

    return data_length;
}

auto curl_xfer_info(
    void*      clientp,
    curl_off_t download_total_bytes,
    curl_off_t download_now_bytes,
    curl_off_t upload_total_bytes,
    curl_off_t upload_now_bytes) -> int
{
    const auto* executor_ptr = static_cast<const executor*>(clientp);

    if (executor_ptr != nullptr && executor_ptr->m_request->m_on_transfer_progress_handler != nullptr)
    {
        if (executor_ptr->m_request->m_on_transfer_progress_handler(
                *executor_ptr->m_request,
                download_total_bytes,
                download_now_bytes,
                upload_total_bytes,
                upload_now_bytes))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0; // continue the request.
    }
}

} // namespace lift
