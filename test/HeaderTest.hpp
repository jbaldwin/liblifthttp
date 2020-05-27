#include "catch.hpp"

#include <lift/Header.hpp>

TEST_CASE("Header basic")
{
    lift::Header header { "name", "value" };
    REQUIRE(header.HeaderFull() == "name: value");
    REQUIRE(header.Name() == "name");
    REQUIRE(header.Value() == "value");
}

TEST_CASE("Header no value")
{
    lift::Header header { "name", "" };
    REQUIRE(header.HeaderFull() == "name: ");
    REQUIRE(header.Name() == "name");
    REQUIRE(header.Value() == "");
}

TEST_CASE("Header request that re-allocates the underlying vector a lot")
{
    constexpr size_t N_HEADERS = 65'000; // lets make a lot of headers to re-allocate a few times

    lift::Request request { "http://herpderp.com" };

    for (size_t i = 0; i < N_HEADERS; ++i) {
        auto name = "name" + std::to_string(i);
        auto value = "value" + std::to_string(i);
        request.Header(name, value);
    }

    size_t idx = 0;
    for (const auto& header : request.Headers()) {
        auto idx_str = std::to_string(idx);
        auto name = "name" + idx_str;
        auto value = "value" + idx_str;
        auto header_full = name + ": " + value;

        REQUIRE(header.HeaderFull() == header_full);
        REQUIRE(header.Name() == name);
        REQUIRE(header.Value() == value);

        ++idx;
    }
}

TEST_CASE("Header from full")
{
    lift::Header header { "name: value" };

    REQUIRE(header.HeaderFull() == "name: value");
    REQUIRE(header.Name() == "name");
    REQUIRE(header.Value() == "value");
}

TEST_CASE("Header lots of allocations in a vector (simulate Response)")
{
    constexpr size_t N_HEADERS = 65'000; // lets make a lot of headers to re-allocate a few times

    std::vector<lift::Header> headers {};

    for (size_t i = 0; i < N_HEADERS; ++i) {
        auto name = "name" + std::to_string(i);
        auto value = "value" + std::to_string(i);
        auto full = name + ": " + value;
        headers.emplace_back(std::move(full));
    }

    size_t idx = 0;
    for (const auto& header : headers) {
        auto idx_str = std::to_string(idx);
        auto name = "name" + idx_str;
        auto value = "value" + idx_str;
        auto header_full = name + ": " + value;

        REQUIRE(header.HeaderFull() == header_full);
        REQUIRE(header.Name() == name);
        REQUIRE(header.Value() == value);

        ++idx;
    }
}

TEST_CASE("Header parsing : from full strings")
{
    {
        lift::Header header { "name" };
        REQUIRE(header.HeaderFull() == "name: ");
        REQUIRE(header.Name() == "name");
        REQUIRE(header.Value() == "");
    }

    {
        lift::Header header { "name:" };
        REQUIRE(header.HeaderFull() == "name: ");
        REQUIRE(header.Name() == "name");
        REQUIRE(header.Value() == "");
    }

    {
        lift::Header header { "name:x" };
        REQUIRE(header.HeaderFull() == "name: x");
        REQUIRE(header.Name() == "name");
        REQUIRE(header.Value() == "x");
    }

    {
        lift::Header header { "name :  x  " };
        REQUIRE(header.HeaderFull() == "name :  x  ");
        REQUIRE(header.Name() == "name ");
        REQUIRE(header.Value() == " x  ");
    }
}
