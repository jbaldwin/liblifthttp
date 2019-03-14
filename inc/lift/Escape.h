#pragma once

#include <string_view>

namespace lift {

/**
 * @param data Data to HTTP escape.
 * @return An HTTP escaped representation of data.
 */
auto escape(
    std::string_view data) -> std::string;

/**
 * @param escaped_data Data to HTTP unescape recursively until it has nothing left to unescape.
 * @return An HTTP unescaped representation of the data.
 */
auto unescape_recurse(
    std::string escaped_data) -> std::string;

/**
 * @param escaped_data Data to HTTP unescape.
 * @return An HTTP unescaped representation of the data.
 */
auto unescape(
    std::string_view escaped_data) -> std::string;

} // lift
