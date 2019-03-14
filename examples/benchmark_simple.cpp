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
    std::cout << program_name << "<url> <duration_seconds> <connections> <threads>" << std::endl;
}

static std::atomic<uint64_t> g_success { 0 };
static std::atomic<uint64_t> g_error { 0 };

static auto on_complete(lift::Request request, lift::EventLoop& event_loop) -> void
{
    if (request->GetCompletionStatus() == lift::RequestStatus::SUCCESS) {
        ++g_success;
    } else {
        ++g_error;
    }

    // And request again!
    if (!event_loop.StartRequest(std::move(request))) {
        std::cerr << "Event loop is no longer accepting requests.\n";
    }
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
    lift::initialize();

    std::vector<std::unique_ptr<lift::EventLoop>> loops;
    for (uint64_t i = 0; i < threads; ++i) {
        auto event_loop_ptr = std::make_unique<lift::EventLoop>();
        auto& request_pool = event_loop_ptr->GetRequestPool();

        for (uint64_t j = 0; j < connections; ++j) {
            auto& event_loop = *event_loop_ptr;

            /**
             * An example using std::bind().
             */
            //using namespace std::placeholders;
            //auto callback = std::bind(on_complete, _1, std::ref(event_loop));
            //auto request = request_pool.Produce(url, std::move(callback), 1000ms);

            /**
             * An example using a lambda.
             */
            auto request = request_pool.Produce(
                url,
                [&event_loop](lift::Request r) {
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

    for (auto& loop : loops) {
        loop->Stop();
    }

    std::cout << "Total success:" << g_success << " total error:" << g_error << std::endl;

    lift::cleanup();

    return 0;
}
