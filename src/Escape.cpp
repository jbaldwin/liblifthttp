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
        std::string value{escaped_data};
        curl_free(escaped_data);
        return value;
    }

    return std::string{};
}

auto unescape_recurse(
    std::string escaped_data
) -> std::string
{
    std::string unescaped_data = unescape(escaped_data);
    while(unescaped_data != escaped_data)
    {
        escaped_data   = std::move(unescaped_data);
        unescaped_data = unescape(escaped_data);
    }
    return unescaped_data;
}

auto unescape(
    std::string_view escaped_data
) -> std::string
{
    char* decoded_data = curl_unescape(
        escaped_data.data(),
        static_cast<int32_t>(escaped_data.length())
    );

    if(decoded_data != nullptr)
    {
        std::string value{decoded_data};
        curl_free(decoded_data);
        return value;
    }

    return std::string{};
}

} // lift
