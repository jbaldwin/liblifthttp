#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("mime_field field_value")
{
    lift::mime_field mime{"name", std::string{"value"}};

    REQUIRE(mime.name() == "name");
    REQUIRE(std::holds_alternative<std::string>(mime.value()));
    auto value = std::get<std::string>(mime.value());
    REQUIRE(value == "value");
}

TEST_CASE("mime_field field_filepath")
{
    lift::mime_field mime{"name2", std::filesystem::path{"/var/log/lift.log"}};

    REQUIRE(mime.name() == "name2");
    REQUIRE(std::holds_alternative<std::filesystem::path>(mime.value()));
    auto path = std::get<std::filesystem::path>(mime.value());
    REQUIRE(path.string() == "/var/log/lift.log");
}

TEST_CASE("mime_field added to request")
{
    lift::mime_field mime1{"name1", std::string{"value"}};
    lift::mime_field mime2{"name2", std::filesystem::path{"/var/log/lift.log"}};

    lift::request request{"http://derp.com"};

    request.mime_field(std::move(mime1));
    request.mime_field(std::move(mime2));

    const auto& mime_fields = request.mime_fields();
    REQUIRE(mime_fields.size() == 2);
    REQUIRE(mime_fields[0].name() == "name1");
    REQUIRE(std::holds_alternative<std::string>(mime_fields[0].value()));
    REQUIRE(std::get<std::string>(mime_fields[0].value()) == "value");
    REQUIRE(mime_fields[1].name() == "name2");
    REQUIRE(std::holds_alternative<std::filesystem::path>(mime_fields[1].value()));
    REQUIRE(std::get<std::filesystem::path>(mime_fields[1].value()) == "/var/log/lift.log");
}
