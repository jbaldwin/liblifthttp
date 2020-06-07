#include <lift/Lift.hpp>

#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static auto print_usage(
    const std::string& program_name) -> void
{
    std::cout << program_name << " <url> <duration_seconds> <connections> <threads>" << std::endl;
}

static auto print_stats(
    std::chrono::seconds duration,
    uint64_t threads,
    uint64_t total_success,
    uint64_t total_error) -> void
{
    auto total = total_success + total_error;
    std::cout << "Thread Stats    Avg\n";
    std::cout << "  Req/sec     " << (total / static_cast<double>(threads) / duration.count()) << "\n";

    std::cout << "Global Stats\n";
    std::cout << "  " << total << " requests in " << duration.count() << "s\n";
    if (total_error > 0) {
        std::cout << "  " << total_error << " errors\n";
    }
    std::cout << "  Req/sec: " << (total / static_cast<double>(duration.count())) << "\n";
}

int main(int argc, char* argv[])
{
    if (argc < 5) {
        print_usage(argv[0]);
        return 0;
    }

    using namespace std::chrono_literals;

    std::string url(argv[1]);
    auto duration = std::chrono::seconds { std::stoul(argv[2]) };
    uint64_t connections = std::stoul(argv[3]);
    uint64_t threads = std::stoul(argv[4]);

    std::atomic<uint64_t> success { 0 };
    std::atomic<uint64_t> error { 0 };

    {
        std::vector<std::unique_ptr<lift::EventLoop>> loops;
        for (uint64_t i = 0; i < threads; ++i) {
            auto event_loop_ptr = std::make_unique<lift::EventLoop>();

            for (uint64_t j = 0; j < connections; ++j) {
                auto& event_loop = *event_loop_ptr;

                auto request_ptr = lift::Request::make_unique(
                    url,
                    1s,
                    [&event_loop, &success, &error](lift::RequestPtr req_ptr, lift::Response response) {
                        if (response.LiftStatus() == lift::LiftStatus::SUCCESS) {
                            ++success;
                        } else {
                            ++error;
                        }

                        // And request again until we are shutting down.
                        event_loop.StartRequest(std::move(req_ptr));
                    });

                request_ptr->FollowRedirects(false);
                request_ptr->Header("Connection", "Keep-Alive");
                event_loop_ptr->StartRequest(std::move(request_ptr));
            }

            loops.emplace_back(std::move(event_loop_ptr));
        }

        std::this_thread::sleep_for(duration);

        for (auto& thread : loops) {
            thread->Stop();
        }
    }

    print_stats(duration, threads, success, error);

    return 0;
}
