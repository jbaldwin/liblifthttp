#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>

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
 *
 * Note that the user is responsible for the lifetime of all string_view's
 * passed into the query builder.  They must be 'alive' until Build() is called.
 */
class query_builder
{
public:
    query_builder() = default;

    /**
     * Sets the scheme for the url.
     *
     * Do not include the :// as the builder will include it for you.
     *
     * @param scheme Examples are "http" or "https".
     * @return query_builder
     */
    auto scheme(std::string_view scheme) -> query_builder&;

    /**
     * Sets the hostname for the url.
     * @param hostname Examples are www.example.com or google.com.  Note that the builder
     *                 currently will not inject a www. prefix so if you want it then make
     *                 sure it is already there.
     * @return query_builder
     */
    auto hostname(std::string_view hostname) -> query_builder&;

    /**
     * Sets the port.
     * @param port The url port.
     * @return query_builder
     */
    auto port(uint16_t port) -> query_builder&;

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
     * @return query_builder
     */
    auto append_path_part(std::string_view path_part) -> query_builder&;

    /**
     * Adds a query parameter to the url.
     *
     * This function does not de-duplicate query parameters.
     * This function will automatically url/http escape the query parameter values.
     * This function will append the query parameters in the same order they are provided in.
     *
     * @param name The name of the parameter.
     * @param value The unescaped value of the parameter.
     * @return query_builder
     */
    auto append_query_parameter(std::string_view name, std::string_view value) -> query_builder&;

    /**
     * Sets the fragment for the url.
     * @param fragment #imafragment
     * @return query_builder
     */
    auto fragment(std::string_view fragment) -> query_builder&;

    /**
     * This function will build the HTTP query string based on the provided
     * information since the last time Build() was called.  Internally each
     * time Build() is called the query builder resets its state for the
     * next query.
     * @return Builds the query into a well formed string.
     */
    auto build() -> std::string;

private:
    /// A buffer for generating the url from its parts.
    std::stringstream m_query;

    /// The url scheme.
    std::string_view m_scheme;
    /// The url hostname.
    std::string_view m_hostname;
    /// The url port.
    uint16_t m_port{0};
    /// The path parts in order.
    std::vector<std::string_view> m_path_parts;
    /// The query parameters (unescaped), they are escaped in Build().
    std::vector<std::pair<std::string_view, std::string_view>> m_query_parameters;
    /// The url fragment.
    std::string_view m_fragment;

    /**
     * Resets all internal member fields to generate a new query.
     */
    auto reset() -> void;
};

} // namespace lift
