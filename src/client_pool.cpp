#include "lift/client_pool.hpp"

namespace lift
{

client_pool::client_pool(options opts) : m_on_thread_callback(opts.on_thread_callback)
{
    for (std::size_t i = 0; i < opts.client_count; ++i)
    {
        auto options = lift::client::options{.on_thread_callback = m_on_thread_callback};

        m_clients.emplace_back(std::make_unique<lift::client>(options));
    }
}

client_pool::~client_pool()
{
    stop();
}

client_pool::client_pool(client_pool&& other)
{
    m_index              = other.m_index.exchange(0);
    m_clients            = std::move(other.m_clients);
    m_on_thread_callback = other.m_on_thread_callback;
}

auto client_pool::stop() -> void
{
    for (auto& client : m_clients)
    {
        client->stop();
    }
}

auto client_pool::size() const -> std::size_t
{
    std::size_t total{0};

    for (const auto& client : m_clients)
    {
        total += client->size();
    }

    return total;
}

auto client_pool::start_request(request_ptr&& request_ptr) -> request::async_future_type
{
    auto index = client_index_advance();
    return m_clients[index]->start_request(std::move(request_ptr));
}

auto client_pool::start_request(request_ptr&& request_ptr, request::async_callback_type callback) -> void
{
    auto index = client_index_advance();
    m_clients[index]->start_request(std::move(request_ptr), std::move(callback));
}

} // namespace lift
