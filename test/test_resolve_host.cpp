#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("resolve_host synchronous perform")
{
    lift::resolve_host rhost{"testhostname", nginx_port, service_ip_address};

    REQUIRE(rhost.host() == "testhostname");
    REQUIRE(rhost.port() == nginx_port);
    REQUIRE(rhost.ip_addr() == service_ip_address);

    lift::request request{"http://testhostname:" + nginx_port_str + "/"};

    request.resolve_host(std::move(rhost));

    auto response = request.perform();
    REQUIRE(response.lift_status() == lift::lift_status::success);
    REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
}

TEST_CASE("resolve_host client")
{
    std::vector<lift::resolve_host> rhosts{
        lift::resolve_host{"testhostname", nginx_port, service_ip_address},
        lift::resolve_host{"herpderp.com", nginx_port, service_ip_address}};

    lift::client client{lift::client::options{.resolve_hosts = std::move(rhosts)}};

    auto on_complete = [&](lift::request_ptr, lift::response response) -> void {
        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    };

    std::vector<lift::request_ptr> requests;
    requests.emplace_back(
        lift::request::make_unique("testhostname:" + nginx_port_str, std::chrono::seconds{60}, on_complete));
    requests.emplace_back(
        lift::request::make_unique("herpderp.com:" + nginx_port_str, std::chrono::seconds{60}, on_complete));

    REQUIRE(client.start_requests(std::move(requests)));
}
