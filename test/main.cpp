#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <lift/Lift.hpp>

static lift::GlobalScopeInitializer g_lift_gsi{};

#include "EscapeTest.hpp"
#include "SyncRequestTest.hpp"
#include "AsyncRequestTest.hpp"
