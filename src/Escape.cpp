#include "lift/Escape.h"

#include <curl/curl.h>

#include <string>

namespace lift
{

auto escape(
    std::string_view data
) -> std::string
{
    char* escaped_data = curl_escape(
        data.data(),
        static_cast<int32_t>(data.length())
    );

    if(escaped_data != nullptr)
    {
        std::string value(escaped_data);
        curl_free(escaped_data);
        return value;
    }

    return "";
}

auto unescape(
    std::string_view data
) -> std::string
{
    char* decoded_data = curl_unescape(
        data.data(),
        static_cast<int32_t>(data.length())
    );

    if(decoded_data != nullptr)
    {
        std::string value(decoded_data);
        curl_free(decoded_data);
        return value;
    }

    return "";
}

} // lift
