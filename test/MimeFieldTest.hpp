#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("MimeField field_value")
{
    lift::MimeField mime { "name", std::string { "value" } };

    REQUIRE(mime.Name() == "name");
    REQUIRE(std::holds_alternative<std::string>(mime.Value()));
    auto value = std::get<std::string>(mime.Value());
    REQUIRE(value == "value");
}

TEST_CASE("MimeField field_filepath")
{
    lift::MimeField mime { "name2", std::filesystem::path { "/var/log/lift.log" } };

    REQUIRE(mime.Name() == "name2");
    REQUIRE(std::holds_alternative<std::filesystem::path>(mime.Value()));
    auto path = std::get<std::filesystem::path>(mime.Value());
    REQUIRE(path.string() == "/var/log/lift.log");
}

TEST_CASE("MimeField added to Request")
{
    lift::MimeField mime1 { "name1", std::string { "value" } };
    lift::MimeField mime2 { "name2", std::filesystem::path { "/var/log/lift.log" } };

    lift::Request request { "http://derp.com" };

    request.MimeField(std::move(mime1));
    request.MimeField(std::move(mime2));

    const auto& mime_fields = request.MimeFields();
    REQUIRE(mime_fields.size() == 2);
    REQUIRE(mime_fields[0].Name() == "name1");
    REQUIRE(std::holds_alternative<std::string>(mime_fields[0].Value()));
    REQUIRE(std::get<std::string>(mime_fields[0].Value()) == "value");
    REQUIRE(mime_fields[1].Name() == "name2");
    REQUIRE(std::holds_alternative<std::filesystem::path>(mime_fields[1].Value()));
    REQUIRE(std::get<std::filesystem::path>(mime_fields[1].Value()) == "/var/log/lift.log");
}