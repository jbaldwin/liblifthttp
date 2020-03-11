#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <lift/Lift.hpp>

static lift::GlobalScopeInitializer g_lift_gsi {};

#include "AsyncRequestTest.hpp"
#include "EscapeTest.hpp"
#include "QueryBuilderTest.hpp"
#include "SyncRequestTest.hpp"
#include "TimesupTest.hpp"
#include "TransferProgressRequest.hpp"
#include "UserDataRequestTest.hpp"