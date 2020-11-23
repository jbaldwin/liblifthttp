#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("header basic")
{
    lift::header h{"name", "value"};
    REQUIRE(h.data() == "name: value");
    REQUIRE(h.name() == "name");
    REQUIRE(h.value() == "value");
}

TEST_CASE("header no value")
{
    lift::header h{"name", ""};
    REQUIRE(h.data() == "name: ");
    REQUIRE(h.name() == "name");
    REQUIRE(h.value() == "");
}

TEST_CASE("header request that re-allocates the underlying vector a lot")
{
    constexpr size_t N_HEADERS = 65'000; // lets make a lot of headers to re-allocate a few times

    lift::request request{"http://herpderp.com"};

    for (size_t i = 0; i < N_HEADERS; ++i)
    {
        auto name  = "name" + std::to_string(i);
        auto value = "value" + std::to_string(i);
        request.header(name, value);
    }

    size_t idx = 0;
    for (const auto& header : request.headers())
    {
        auto idx_str     = std::to_string(idx);
        auto name        = "name" + idx_str;
        auto value       = "value" + idx_str;
        auto header_full = name + ": " + value;

        REQUIRE(header.data() == header_full);
        REQUIRE(header.name() == name);
        REQUIRE(header.value() == value);

        ++idx;
    }
}

TEST_CASE("header from full")
{
    lift::header h{"name: value"};

    REQUIRE(h.data() == "name: value");
    REQUIRE(h.name() == "name");
    REQUIRE(h.value() == "value");
}

TEST_CASE("header lots of allocations in a vector (simulate response)")
{
    constexpr size_t N_HEADERS = 65'000; // lets make a lot of headers to re-allocate a few times

    std::vector<lift::header> headers{};

    for (size_t i = 0; i < N_HEADERS; ++i)
    {
        auto name  = "name" + std::to_string(i);
        auto value = "value" + std::to_string(i);
        auto full  = name + ": " + value;
        headers.emplace_back(std::move(full));
    }

    size_t idx = 0;
    for (const auto& header : headers)
    {
        auto idx_str     = std::to_string(idx);
        auto name        = "name" + idx_str;
        auto value       = "value" + idx_str;
        auto header_full = name + ": " + value;

        REQUIRE(header.data() == header_full);
        REQUIRE(header.name() == name);
        REQUIRE(header.value() == value);

        ++idx;
    }
}

TEST_CASE("header parsing : from full strings")
{
    {
        lift::header h{"name"};
        REQUIRE(h.data() == "name: ");
        REQUIRE(h.name() == "name");
        REQUIRE(h.value() == "");
    }

    {
        lift::header h{"name:"};
        REQUIRE(h.data() == "name: ");
        REQUIRE(h.name() == "name");
        REQUIRE(h.value() == "");
    }

    {
        lift::header h{"name:x"};
        REQUIRE(h.data() == "name: x");
        REQUIRE(h.name() == "name");
        REQUIRE(h.value() == "x");
    }

    {
        lift::header h{"name :  x  "};
        REQUIRE(h.data() == "name :  x  ");
        REQUIRE(h.name() == "name ");
        REQUIRE(h.value() == " x  ");
    }
}
