#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

static auto user_data_on_complete(lift::RequestHandle request, uint64_t user_data_value1, double user_data_value2) -> void
{
    if(user_data_value1 == 1)
    {
        REQUIRE(user_data_value2 == 100.5);
    }
    else if(user_data_value1 == 2)
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
    lift::EventLoop event_loop{};

    // Technically can hard code in this instance for the lambda captures, but to make it a bit
    // more like an example we'll include a unique "request_id" that gets captured as the user data.
    uint64_t request_id = 1;

    auto& request_pool = event_loop.GetRequestPool();
    auto req1 = request_pool.Produce("http://localhost:80/", std::chrono::seconds{1});
    req1->SetOnCompleteHandler([request_id](lift::RequestHandle r) { user_data_on_complete(std::move(r), request_id, 100.5); });
    event_loop.StartRequest(std::move(req1));

    request_id = 2;

    auto req2 = request_pool.Produce("http://localhost:80/", std::chrono::seconds{1});
    req2->SetOnCompleteHandler([request_id](lift::RequestHandle r) { user_data_on_complete(std::move(r), request_id, 1234.567); });
    event_loop.StartRequest(std::move(req2));

    while (event_loop.GetActiveRequestCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
}