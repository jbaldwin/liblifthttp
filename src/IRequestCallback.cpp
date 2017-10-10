#include "lift/IRequestCallback.h"

namespace lift
{

auto IRequestCallback::GetEventLoop() -> EventLoop&
{
    return *m_event_loop;
}

auto IRequestCallback::GetEventLoop() const -> const EventLoop&
{
    return *m_event_loop;
}

} // lift
