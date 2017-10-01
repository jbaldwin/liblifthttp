#include "lift/Escape.h"

#include <curl/curl.h>

namespace lift
{

auto escape(
    StringView data
) -> std::string
{
    char* escaped_data = curl_escape(
        data.data(),
        static_cast<int32_t>(data.length())
    );

    if(escaped_data != nullptr)
    {
        auto value = std::string(escaped_data);
        curl_free(escaped_data);
        return value;
    }

    return "";
}

auto unescape(
    StringView data
) -> std::string
{
    char* decoded_data = curl_unescape(
        data.data(),
        static_cast<int32_t>(data.length())
    );

    if(decoded_data != nullptr)
    {
        auto value = std::string(decoded_data);
        curl_free(decoded_data);
        return value;
    }

    return "";
}

} // lift
