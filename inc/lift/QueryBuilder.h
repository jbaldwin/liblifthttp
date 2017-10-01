#pragma once

#include "lift/Types.h"

#include <string>
#include <sstream>
#include <vector>

namespace lift
{

/**
 * The query builder is a simple url builder class to reduce string allocations
 * and copies.
 *
 * Once all the url fields are set call Build() to generate the url.  The fields are then
 * reset and the builder can be re-used to craft another url re-using the existing buffers.
 *
 * It currently does not validate that the correct parts for a url are provided,
 * so the user must be diligent to set all the appropriate fields.
 */
class QueryBuilder
{
public:
    QueryBuilder();

    /**
     * Sets the scheme for the url.
     *
     * Do not include the :// as the builder will include it for you.
     *
     * @param scheme Examples are "http" or "https".
     * @return QueryBuilder
     */
    auto SetScheme(
        StringView scheme
    ) -> QueryBuilder&;

    /**
     * Sets the hostname for the url.
     * @param hostname Examples are www.example.com or google.com.  Note that the builder
     *                 currently will not inject a www. prefix so if you want it then make
     *                 sure it is already there.
     * @return QueryBuilder
     */
    auto SetHostname(
        StringView hostname
    ) -> QueryBuilder&;

    /**
     * Sets the port.
     *
     * The SetPort(uint16_t) function takes precendence if both are called.
     *
     * @param port The url port.
     * @return QueryBuilder
     */
    auto SetPort(
        StringView port
    ) -> QueryBuilder&;

    /**
     * Sets the port.
     *
     * This function takes precedence over the SetPort(StringView) version if both are set.
     *
     * @param port The url port.
     * @return QueryBuidler
     */
    auto SetPort(
        uint16_t port
    ) -> QueryBuilder&;

    /**
     * Adds a path part to the url.  Path parts shouldn't include '/' as the builder
     * will appropriately put them in the correct places.
     *
     * If the path was /test/path/parts then this function should be called three times
     * with the parameters 'test' 'path' and 'parts' in that order.
     *
     * This function will append the path parts in the same order they are provided in.
     *
     * @param path_part The path part to add to the query.
     * @return QueryBuilder
     */
    auto AppendPathPart(
        StringView path_part
    ) -> QueryBuilder&;

    /**
     * Adds a query parameter to the url.
     *
     * This function does not de-duplicate query parameters.
     * This function will automatically url/http escape the query parameter values.
     * This function will append the query parameters in the same order they are provided in.
     *
     * @param name The name of the parameter.
     * @param value The unescaped value of the parameter.
     * @return QueryBuilder
     */
    auto AppendPathPart(
        StringView name,
        StringView value
    ) -> QueryBuilder&;

    /**
     * Sets the fragment for the url.
     * @param fragment #imafragment
     * @return QueryBuilder
     */
    auto SetFragment(
        StringView fragment
    ) -> QueryBuilder&;

    auto Build() -> std::string;

private:
    std::stringstream m_converter;  ///< Used to convert the m_port_int value.
    std::string m_query;            ///< A buffer for generating the url from its parts.

    StringView m_scheme;            ///< The url scheme.
    StringView m_hostname;          ///< The url hostname.
    StringView m_port_str;          ///< The url port (as a string).
    uint16_t m_port_int;            ///< The url port (as an integer).
    std::vector<StringView> m_path_parts;   ///< The path parts in order.
    std::vector<
        std::pair<StringView, StringView>
    > m_query_parameters;           ///< The query paremeters (unescaped).
    StringView m_fragment;          ///< The url fragment.

    auto reset() -> void;
};

} // lift
