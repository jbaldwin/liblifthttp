#include <lift/lift.hpp>

#include <atomic>
#include <chrono>
#include <getopt.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

static auto print_usage(const std::string& program_name) -> void
{
    std::cout << "Usage: " << program_name << "<options> <url>\n";
    std::cout << "    -c --connections  HTTP Connections to use per thread.\n";
    std::cout << "    -t --threads      Number of threads to use.\n";
    std::cout << "                      evenly between each worker thread.\n";
    std::cout << "    -d --duration     Duration of the test in seconds\n";
    std::cout << "    -h --help         Print this help usage.\n";
}

static auto print_stats(std::chrono::seconds duration, uint64_t threads, uint64_t total_success, uint64_t total_error)
    -> void
{
    auto total = total_success + total_error;
    std::cout << "Thread Stats    Avg\n";
    std::cout << "  Req/sec     " << (total / static_cast<double>(threads) / duration.count()) << "\n";

    std::cout << "Global Stats\n";
    std::cout << "  " << total << " requests in " << duration.count() << "s\n";
    if (total_error > 0)
    {
        std::cout << "  " << total_error << " errors\n";
    }
    std::cout << "  Req/sec: " << (total / static_cast<double>(duration.count())) << "\n";
}

int main(int argc, char* argv[])
{
    constexpr char   short_options[] = "c:d:t:h";
    constexpr option long_options[]  = {
         {"help", no_argument, nullptr, 'h'},
         {"connections", required_argument, nullptr, 'c'},
         {"duration", required_argument, nullptr, 'd'},
         {"threads", required_argument, nullptr, 't'},
         {nullptr, 0, nullptr, 0}};

    int option_index = 0;
    int opt          = 0;

    std::optional<uint64_t>             connections_opt;
    std::optional<std::chrono::seconds> duration_opt;
    std::optional<uint64_t>             threads_opt;
    std::optional<std::string>          url_opt;

    std::size_t index{0};

    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_usage(argv[0]);
                return EXIT_SUCCESS;
            case 'c':
                connections_opt = std::stoul(optarg);
                break;
            case 'd':
                duration_opt = std::chrono::seconds{std::stol(optarg)};
                break;
            case 't':
                threads_opt = std::stoul(optarg);
                break;
        }

        index += 2;
    }

    if (index + 1 < argc)
    {
        url_opt = argv[index + 1];
    }

    if (!connections_opt.has_value() || !duration_opt.has_value() || !threads_opt.has_value() || !url_opt.has_value())
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    using namespace std::chrono_literals;

    auto url         = url_opt.value();
    auto duration    = duration_opt.value();
    auto connections = connections_opt.value();
    auto threads     = threads_opt.value();

    std::cout << "Running " << duration.count() << "s test @ " << url << "\n";

    std::atomic<uint64_t> success{0};
    std::atomic<uint64_t> error{0};

    {
        std::vector<lift::request::async_callback_type> callbacks;
        callbacks.reserve(threads);
        std::vector<std::unique_ptr<lift::client>> clients;
        clients.reserve(threads);

        for (uint64_t i = 0; i < threads; ++i)
        {
            clients.emplace_back(std::make_unique<lift::client>());

            callbacks.emplace_back(
                [&clients, &success, &error, &callbacks, i](lift::request_ptr req_ptr, lift::response response)
                {
                    if (response.lift_status() == lift::lift_status::success)
                    {
                        success.fetch_add(1, std::memory_order_relaxed);
                    }
                    else if (response.lift_status() == lift::lift_status::error_failed_to_start)
                    {
                        return;
                    }
                    else
                    {
                        error.fetch_add(1, std::memory_order_relaxed);
                    }

                    // And request again until we are shutting down.
                    auto copy_callback = callbacks[i];
                    clients[i]->start_request(std::move(req_ptr), std::move(copy_callback));
                });

            for (uint64_t j = 0; j < connections; ++j)
            {
                auto request_ptr = std::make_unique<lift::request>(url, 30s);

                request_ptr->follow_redirects(false);
                request_ptr->header("Connection", "Keep-Alive");
                auto copy_callback = callbacks[i];
                clients[i]->start_request(std::move(request_ptr), std::move(copy_callback));
            }
        }

        std::this_thread::sleep_for(duration);

        for (auto& thread : clients)
        {
            thread->stop();
        }
    }

    print_stats(duration, threads, success, error);

    return 0;
}
