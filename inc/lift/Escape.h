#pragma once

#include "lift/Types.h"

namespace lift
{

/**
 * @param data Data to HTTP escape.
 * @return An HTTP escaped representation of data.
 */
auto escape(
    StringView data
) -> std::string;

/**
 * @param data Data to HTTP unescape.
 * @return An HTTP unescaped representation of the data.
 */
auto unescape(
    StringView data
) -> std::string;

} // lift
