#include <lift/Lift.h>

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
    uint64_t duration_s,
    uint64_t threads,
    uint64_t total_success,
    uint64_t total_error) -> void
{
    auto total = total_success + total_error;
    std::cout << "  Thread Stats    Avg\n";
    std::cout << "    Req/sec     " << (total / static_cast<double>(threads) / duration_s) << "\n";

    std::cout << "  " << total << " requests in " << duration_s << "s\n";
    if (total_error > 0) {
        std::cout << "  " << total_error << " errors\n";
    }
    std::cout << "Requests/sec: " << (total / static_cast<double>(duration_s)) << "\n";
}

static std::atomic<uint64_t> g_success{ 0 };
static std::atomic<uint64_t> g_error{ 0 };

static auto on_complete(lift::RequestHandle request, lift::EventLoop& event_loop) -> void
{
    if (request->GetCompletionStatus() == lift::RequestStatus::SUCCESS) {
        ++g_success;
    } else {
        ++g_error;
    }

    // And request again until we are shutting down.
    event_loop.StartRequest(std::move(request));
}

int main(int argc, char* argv[])
{
    if (argc < 5) {
        print_usage(argv[0]);
        return 0;
    }

    using namespace std::chrono_literals;

    std::string url(argv[1]);
    uint64_t duration_s = std::stoul(argv[2]);
    uint64_t connections = std::stoul(argv[3]);
    uint64_t threads = std::stoul(argv[4]);

    // Initialize must be called first before using the LiftHttp library.
    lift::GlobalScopeInitializer lift_init{};

    {
        std::vector<std::unique_ptr<lift::EventLoop>> loops;
        for (uint64_t i = 0; i < threads; ++i) {
            auto event_loop_ptr = std::make_unique<lift::EventLoop>();
            auto& request_pool = event_loop_ptr->GetRequestPool();

            for (uint64_t j = 0; j < connections; ++j) {
                auto& event_loop = *event_loop_ptr;

                auto request = request_pool.Produce(
                    url,
                    [&event_loop](lift::RequestHandle r) {
                        on_complete(std::move(r), event_loop);
                    },
                    1s);
                request->SetFollowRedirects(false);
                request->AddHeader("Connection", "Keep-Alive");
                event_loop_ptr->StartRequest(std::move(request));
            }

            loops.emplace_back(std::move(event_loop_ptr));
        }

        std::chrono::seconds seconds(duration_s);
        std::this_thread::sleep_for(seconds);

        for (auto& thread : loops) {
            thread->Stop();
        }
    }

    print_stats(duration_s, threads, g_success, g_error);

    return 0;
}
