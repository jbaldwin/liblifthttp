#pragma once

#include "lift/client.hpp"

#include <vector>

namespace lift
{

class client_pool
{
public:
    using on_thread_callback_type = std::function<void()>;

    struct options
    {
        /// @brief The number of clients to spin up in the pool.
        std::size_t client_count{2};
        /// @brief If this functor is provided it is called on each client's
        ///        background thread when it starts and stops.
        on_thread_callback_type on_thread_callback{nullptr};
    };

    explicit client_pool(options opts = options{2, nullptr});

    ~client_pool();

    client_pool(const client_pool&) = delete;
    client_pool(client_pool&&);
    auto operator=(const client_pool&) noexcept -> client_pool& = delete;
    auto operator=(client_pool&&) noexcept -> client_pool&;

    auto stop() -> void;

    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto empty() const -> bool { return size() == 0; }

    [[nodiscard]] auto start_request(request_ptr&& request_ptr) -> request::async_future_type;
    auto               start_request(request_ptr&& request_ptr, request::async_callback_type callback) -> void;

    template<typename container_type>
    auto start_requests(container_type&& requests) -> std::vector<request::async_future_type>
    {
        std::vector<request::async_future_type> futures{};
        futures.reserve(std::size(requests));

        for (auto& request_ptr : requests)
        {
            if (request_ptr != nullptr)
            {
                auto index = client_index_advance();
                futures.emplace_back(m_clients[index]->start_request(std::move(request_ptr)));
            }
        }

        return futures;
    }

    template<typename container_type>
    auto start_requests(container_type&& requests, request::async_callback_type callback) -> void
    {
        if (callback == nullptr)
        {
            throw std::runtime_error{"lift::client_pool::start_requests (callback) The callback cannot be nullptr."};
        }

        for (auto& request_ptr : requests)
        {
            if (request_ptr != nullptr)
            {
                auto index = client_index_advance();
                m_clients[index]->start_request(std::move(request_ptr, callback));
            }
        }
    }

private:
    std::atomic<std::size_t>                   m_index{0};
    std::vector<std::unique_ptr<lift::client>> m_clients{};
    on_thread_callback_type                    m_on_thread_callback{nullptr};

    auto client_index_advance() -> std::size_t
    {
        return m_index.fetch_add(1, std::memory_order_acq_rel) % m_clients.size();
    }
};

} // namespace lift
