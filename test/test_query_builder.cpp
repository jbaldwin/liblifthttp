#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("Query Builder simple")
{
    lift::query_builder qb{};
    qb.scheme("https")
        .hostname("www.example.com")
        .port(443)
        .append_path_part("test")
        .append_path_part("path")
        .append_query_parameter("param1", "value1")
        .append_query_parameter("param2", "value2");

    auto url = qb.build();

    REQUIRE(url == "https://www.example.com:443/test/path?param1=value1&param2=value2");

    qb.scheme("http").hostname("www.reddit.com").append_path_part("r").append_path_part("funny");
    url = qb.build();

    REQUIRE(url == "http://www.reddit.com/r/funny");
}
