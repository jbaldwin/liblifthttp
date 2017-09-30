#pragma once

#include "lift/Types.h"

namespace lift
{

class Header
{
public:
    /**
     * Creates a view into the header data.
     * @param header_data The full header data to split into name/value pair.
     */
    explicit Header(
        StringView header_data
    );

    /**
     * @return A view into the header's name.
     */
    auto GetName() const -> StringView;

    /**
     * @return True if this header has a value.
     */
    auto HasValue() const -> bool;

    /**
     * @return A view into the header's value or null if it doesn't have a value.
     */
    auto GetValue() const -> StringView;

private:
    StringView m_name;  ///< The header's name.
    StringView m_value; ///< The header's value if it has one.
};

} // lift
