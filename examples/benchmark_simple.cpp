#include <lift/Lift.h>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

static auto print_usage(
    const std::string& program_name
) -> void
{
    std::cout << program_name << "<url> <duration_seconds> <connections> <threads>" << std::endl;
}

std::atomic<uint64_t> g_success{0};
std::atomic<uint64_t> g_error{0};

auto on_complete(lift::Request request) -> void
{
    if(request->GetCompletionStatus() == lift::RequestStatus::SUCCESS)
    {
        ++g_success;
    }
    else
    {
        ++g_error;
    }

    // And request again!
    auto* event_loop = static_cast<lift::EventLoop*>(request->GetUserData());
    if(!event_loop->StartRequest(std::move(request)))
    {
        std::cerr << "Event loop is no longer accepting requests.\n";
    }
}

int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        print_usage(argv[0]);
        return 0;
    }

    using namespace std::chrono_literals;

    std::string url(argv[1]);
    uint64_t duration_s  = std::stoul(argv[2]);
    uint64_t connections = std::stoul(argv[3]);
    uint64_t threads     = std::stoul(argv[4]);

    // Initialize must be called first before using the LiftHttp library.
    lift::initialize();

    std::vector<std::unique_ptr<lift::EventLoop>> loops;
    for(uint64_t i = 0; i < threads; ++i)
    {
        auto event_loop = std::make_unique<lift::EventLoop>();
        auto& request_pool = event_loop->GetRequestPool();

        for(uint64_t j = 0; j < connections; ++j)
        {
            auto request = request_pool.Produce(url, on_complete, 1000ms);
            request->SetFollowRedirects(false);
            request->SetUserData(event_loop.get());
            request->AddHeader("Connection", "Keep-Alive");
            event_loop->StartRequest(std::move(request));
        }

        loops.emplace_back(std::move(event_loop));
    }

    std::chrono::seconds seconds(duration_s);
    std::this_thread::sleep_for(seconds);

    for(auto& loop : loops)
    {
        loop->Stop();
    }

    std::cout << "Total success:" << g_success << " total error:" << g_error << std::endl;

    lift::cleanup();

    return 0;
}
