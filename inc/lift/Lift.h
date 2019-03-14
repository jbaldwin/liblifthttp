#pragma once

#include "lift/EventLoop.h"
#include "lift/QueryBuilder.h"
#include "lift/Request.h"
#include "lift/RequestHandle.h"
#include "lift/RequestPool.h"

namespace lift {

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
