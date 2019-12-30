#pragma once

#include "lift/EventLoop.hpp"

namespace lift {

template <typename Container>
auto EventLoop::StartRequests(
    Container requests) -> bool
{
    if (m_is_stopping) {
        return false;
    }

    // We'll prepare now since it won't block the event loop thread.
    // Since this might not be cheap do it outside the lock
    for (auto& request : requests) {
        request->prepareForPerform();
    }

    m_active_request_count += std::size(requests);

    // Lock scope
    {
        std::lock_guard<std::mutex> guard(m_pending_requests_lock);
        for (auto& request : requests) {
            m_pending_requests.emplace_back(std::move(request));
        }
    }

    // Notify the even loop thread that there are requests waiting to be picked up.
    uv_async_send(&m_async);

    return true;
}

} // lift
