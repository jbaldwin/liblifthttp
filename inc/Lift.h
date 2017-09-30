#pragma once

#include "Request.h"
#include "AsyncRequest.h"
#include "IRequestCb.h"
#include "EventLoop.h"

namespace lift
{

/**
 * Initializes the LiftHttp library, this function should be called once
 * per application and on a single thread.
 */
auto initialize() -> void;

/**
 * Cleans up the LiftHttp library, this function should be called once
 * per application on a single thread when shutting down.
 */
auto cleanup() -> void;

} // lift
