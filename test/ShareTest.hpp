#pragma once

#include "catch.hpp"

#include <lift/Lift.hpp>

TEST_CASE("Share Requests ALL")
{
    auto lift_share_ptr = std::make_shared<lift::Share>(lift::ShareOptions::ALL);

    for(std::size_t i = 0; i < 5; ++i) {
        lift::Request request {
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds { 60 }
        };

        auto response = request.Perform(lift_share_ptr);

        REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
        REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    }
}

TEST_CASE("Share Requests NOTHING")
{
    auto lift_share_ptr = std::make_shared<lift::Share>(lift::ShareOptions::NOTHING);

    for(std::size_t i = 0; i < 5; ++i) {
        lift::Request request {
            "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
            std::chrono::seconds { 60 }
        };

        auto response = request.Perform(lift_share_ptr);

        REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
        REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
    }
}

TEST_CASE("Share EventLoop synchronous")
{
    auto lift_share_ptr = std::make_shared<lift::Share>(lift::ShareOptions::ALL);

    lift::EventLoop ev1 {
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::vector<lift::ResolveHost> {},
        lift_share_ptr
    };

    lift::EventLoop ev2 {
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::vector<lift::ResolveHost> {},
        lift_share_ptr
    };

    auto request1 = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds { 60 },
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        });

    auto request2 = lift::Request::make_unique(
        "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
        std::chrono::seconds { 60 },
        [&](std::unique_ptr<lift::Request>, lift::Response response) {
            REQUIRE(response.LiftStatus() == lift::LiftStatus::SUCCESS);
            REQUIRE(response.StatusCode() == lift::http::StatusCode::HTTP_200_OK);
        });

    ev1.StartRequest(std::move(request1));

    while (ev1.ActiveRequestCount() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    ev2.StartRequest(std::move(request2));

    ev1.Stop();
    ev2.Stop();
}

TEST_CASE("Share EventLoop overlapping requests")
{
    std::atomic<uint64_t> count { 0 };

    constexpr size_t N_SHARE = 1;
    constexpr size_t N_EVENT_LOOPS = 2;
    constexpr size_t N_REQUESTS = 10'000;

    std::vector<std::shared_ptr<lift::Share>> lift_share {};
    for (size_t i = 0; i < N_SHARE; ++i) {
        lift_share.emplace_back(std::make_shared<lift::Share>(lift::ShareOptions::ALL));
    }

    auto worker_func = [&count, &lift_share]() {
        static size_t share_counter { 0 };
        lift::EventLoop event_loop {
            std::nullopt,
            std::nullopt,
            std::nullopt,
            std::vector<lift::ResolveHost> {},
            lift_share[share_counter++ % N_SHARE]
        };

        for (size_t i = 0; i < N_REQUESTS; ++i) {
            auto request_ptr = lift::Request::make_unique(
                "http://" + NGINX_HOSTNAME + ":" + NGINX_PORT_STR + "/",
                std::chrono::seconds { 5 },
                [&](std::unique_ptr<lift::Request>, lift::Response response) {
                    ++count;
                });

            event_loop.StartRequest(std::move(request_ptr));
        }

        while (event_loop.ActiveRequestCount() > 0) {
            // std::cerr << "Active=" << event_loop.ActiveRequestCount() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };

    std::vector<std::thread> workers {};
    for (size_t i = 0; i < N_EVENT_LOOPS; ++i) {
        workers.emplace_back(worker_func);
    }

    for (auto& worker : workers) {
        worker.join();
    }

    REQUIRE(count == N_EVENT_LOOPS * N_REQUESTS);
}