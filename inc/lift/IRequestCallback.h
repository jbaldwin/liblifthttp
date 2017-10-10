#pragma once

#include "lift/RequestPool.h"

namespace lift
{

class EventLoop;

/**
 * Interface for the set of possible asynchronous request callbacks when
 * the request completes/times out/errors.
 */
class IRequestCallback
{
    friend class EventLoop;

public:
    virtual ~IRequestCallback() = default;

    /**
     * @return The EventLoop that owns this request callback structure.
     * @{
     */
    auto GetEventLoop() -> EventLoop&;
    auto GetEventLoop() const -> const EventLoop&;
    /** @} */

    /**
     * Request callback on completion.  The request will have either a success
     * status or an error set.
     * @param request The request that has completed.
     */
    virtual auto OnComplete(
        Request request
    ) -> void = 0;

private:
    EventLoop* m_event_loop;
};

} // lift
