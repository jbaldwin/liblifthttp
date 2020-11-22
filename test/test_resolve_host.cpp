#include "catch.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("ResolveHost synchronous perform")
{
    lift::ResolveHost rhost{"testhostname", NGINX_PORT, SERVICE_IP_ADDRESS};

    REQUIRE(rhost.Host() == "testhostname");
    REQUIRE(rhost.Port() == NGINX_PORT);
    REQUIRE(rhost.IpAddr() == SERVICE_IP_ADDRESS);

    lift::Request request{"http://testhostname:" + NGINX_PORT_STR + "/"};

    request.ResolveHost(std::move(rhost));

    auto response = request.Perform();
    REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
    REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
}

TEST_CASE("ResolveHost event_loop")
{
    std::vector<lift::ResolveHost> rhosts{
        lift::ResolveHost{"testhostname", NGINX_PORT, SERVICE_IP_ADDRESS},
        lift::ResolveHost{"herpderp.com", NGINX_PORT, SERVICE_IP_ADDRESS}};

    lift::event_loop ev{lift::event_loop::options{.resolve_hosts = std::move(rhosts)}};

    auto on_complete = [&](lift::RequestPtr, lift::Response response) -> void {
        REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
        REQUIRE(response.StatusCode() == lift::http::status_code::http_200_ok);
    };

    std::vector<lift::RequestPtr> requests;
    requests.emplace_back(
        lift::Request::make_unique("testhostname:" + NGINX_PORT_STR, std::chrono::seconds{60}, on_complete));
    requests.emplace_back(
        lift::Request::make_unique("herpderp.com:" + NGINX_PORT_STR, std::chrono::seconds{60}, on_complete));

    REQUIRE(ev.start_requests(std::move(requests)));
}
