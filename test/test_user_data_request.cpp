#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

static auto user_data_on_complete(
    lift::request_ptr request_ptr, lift::response response, uint64_t user_data_value1, double user_data_value2) -> void
{
    if (user_data_value1 == 1)
    {
        REQUIRE(user_data_value2 == 100.5);
    }
    else if (user_data_value1 == 2)
    {
        REQUIRE(user_data_value2 == 1234.567);
    }
    else
    {
        // this will fail if the above do not match
        REQUIRE(user_data_value1 == user_data_value2);
    }
}

TEST_CASE("User data")
{
    lift::client client{};

    // Technically can hard code in this instance for the lambda captures, but to make it a bit
    // more like an example we'll include a unique "request_id" that gets captured as the user data.
    uint64_t request_id = 1;

    auto req1 = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{1});
    client.start_request(
        std::move(req1),
        [request_id](lift::request_ptr request, lift::response response)
        { user_data_on_complete(std::move(request), std::move(response), request_id, 100.5); });

    request_id = 2;

    auto req2 = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{1});
    client.start_request(
        std::move(req2),
        [request_id](lift::request_ptr request, lift::response response)
        { user_data_on_complete(std::move(request), std::move(response), request_id, 1234.567); });

    while (!client.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}
