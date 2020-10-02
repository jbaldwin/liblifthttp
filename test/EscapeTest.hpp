#pragma once

#include "catch.hpp"

#include <lift/Escape.hpp>

TEST_CASE("Escape", "[escape]")
{
    auto escaped =
        lift::escape("1234567890-=qwertyuiop[]asdfghjkl;'zxcvbnm,./!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>? ");
    REQUIRE(
        escaped ==
        "1234567890-%3Dqwertyuiop%5B%5Dasdfghjkl%3B%27zxcvbnm%2C.%2F%21%40%23%24%25%5E%26%2A%28%29_%2BQWERTYUIOP%7B%7DASDFGHJKL%3A%22ZXCVBNM%3C%3E%3F%20");
}

TEST_CASE("Unescape")
{
    auto unescaped = lift::unescape(
        "1234567890-%3Dqwertyuiop%5B%5Dasdfghjkl%3B%27zxcvbnm%2C.%2F%21%40%23%24%25%5E%26%2A%28%29_%2BQWERTYUIOP%7B%7DASDFGHJKL%3A%22ZXCVBNM%3C%3E%3F%20");
    REQUIRE(
        unescaped == "1234567890-=qwertyuiop[]asdfghjkl;'zxcvbnm,./!@#$%^&*()_+QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>? ");
}

TEST_CASE("Unescape Recrusive")
{
    auto escaped1  = lift::escape(" ");
    auto escaped2  = lift::escape(escaped1);
    auto escaped3  = lift::escape(escaped2);
    auto unescaped = lift::unescape_recurse(escaped3);

    REQUIRE(escaped1 == "%20");
    REQUIRE(escaped2 == "%2520");
    REQUIRE(escaped3 == "%252520");
    REQUIRE(unescaped == " ");
}
