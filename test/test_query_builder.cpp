#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("Query Builder simple")
{
    lift::QueryBuilder query_builder{};
    query_builder.Scheme("https")
        .Hostname("www.example.com")
        .Port(443)
        .AppendPathPart("test")
        .AppendPathPart("path")
        .AppendQueryParameter("param1", "value1")
        .AppendQueryParameter("param2", "value2");

    auto url = query_builder.Build();

    REQUIRE(url == "https://www.example.com:443/test/path?param1=value1&param2=value2");

    query_builder.Scheme("http").Hostname("www.reddit.com").AppendPathPart("r").AppendPathPart("funny");
    url = query_builder.Build();

    REQUIRE(url == "http://www.reddit.com/r/funny");
}
