namespace lift
{

template<typename Container>
auto EventLoop::StartRequests(
    Container& requests
) -> void
{
    // We'll prepare now since it won't block the event loop thread.
    // Since this might not be cheap do it outside the lock
    for(auto& request : requests)
    {
        request->prepareForPerform();
    }

    // Lock scope
    {
        std::lock_guard<std::mutex> guard(m_pending_requests_lock);
        for(auto& request : requests)
        {
            m_pending_requests.emplace_back(std::move(request));
        }
    }

    uv_async_send(&m_async);
}

} // lift
