#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <lift/Lift.h>

static lift::GlobalScopeInitializer g_lift_gsi{};

TEST_CASE("Simple synchronous", "[test]")
{
    lift::RequestPool request_pool{};
    auto request = request_pool.Produce("http://localhost:80/");
    request->Perform();

    REQUIRE(request->GetCompletionStatus() == lift::RequestStatus::SUCCESS);
    REQUIRE(request->GetResponseStatusCode() == lift::http::StatusCode::HTTP_200_OK);
}