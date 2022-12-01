#include "catch_amalgamated.hpp"
#include "setup.hpp"
#include <lift/lift.hpp>

TEST_CASE("share requests ALL")
{
    auto lift_share_ptr = std::make_shared<lift::share>(lift::share::options::all);

    for (std::size_t i = 0; i < 5; ++i)
    {
        lift::request request{"http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}};

        auto response = request.perform(lift_share_ptr);

        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    }
}

TEST_CASE("share requests NOTHING")
{
    auto lift_share_ptr = std::make_shared<lift::share>(lift::share::options::nothing);

    for (std::size_t i = 0; i < 5; ++i)
    {
        lift::request request{"http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60}};

        auto response = request.perform(lift_share_ptr);

        REQUIRE(response.lift_status() == lift::lift_status::success);
        REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
    }
}

TEST_CASE("share client synchronous")
{
    auto lift_share_ptr = std::make_shared<lift::share>(lift::share::options::all);

    lift::client client1{lift::client::options{.share = lift_share_ptr}};

    lift::client client2{lift::client::options{.share = lift_share_ptr}};

    auto request1 = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});

    auto request2 = std::make_unique<lift::request>(
        "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{60});

    client1.start_request(
        std::move(request1),
        [&](std::unique_ptr<lift::request>, lift::response response)
        {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        });

    while (!client1.empty())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    client2.start_request(
        std::move(request2),
        [&](std::unique_ptr<lift::request>, lift::response response)
        {
            REQUIRE(response.lift_status() == lift::lift_status::success);
            REQUIRE(response.status_code() == lift::http::status_code::http_200_ok);
        });

    client1.stop();
    client2.stop();
}

TEST_CASE("share client overlapping requests")
{
    std::atomic<uint64_t> count{0};

    constexpr size_t N_SHARE       = 1;
    constexpr size_t N_EVENT_LOOPS = 2;
    constexpr size_t N_REQUESTS    = 10'000;

    std::vector<std::shared_ptr<lift::share>> lift_share{};
    for (size_t i = 0; i < N_SHARE; ++i)
    {
        lift_share.emplace_back(std::make_shared<lift::share>(lift::share::options::all));
    }

    auto worker_func = [&count, &lift_share]()
    {
        static size_t share_counter{0};
        lift::client  client{lift::client::options{.share = lift_share[share_counter++ % N_SHARE]}};

        for (size_t i = 0; i < N_REQUESTS; ++i)
        {
            auto request_ptr = std::make_unique<lift::request>(
                "http://" + nginx_hostname + ":" + nginx_port_str + "/", std::chrono::seconds{5});

            client.start_request(
                std::move(request_ptr),
                [&](std::unique_ptr<lift::request>, lift::response response)
                { count.fetch_add(1, std::memory_order_relaxed); });
        }

        while (!client.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::vector<std::thread> workers{};
    for (size_t i = 0; i < N_EVENT_LOOPS; ++i)
    {
        workers.emplace_back(worker_func);
    }

    for (auto& worker : workers)
    {
        worker.join();
    }

    REQUIRE(count == N_EVENT_LOOPS * N_REQUESTS);
}
