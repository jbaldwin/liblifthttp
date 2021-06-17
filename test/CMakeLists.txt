cmake_minimum_required(VERSION 3.0)
project(liblifthttp_tests)

option(LIFT_LOCALHOST_TESTS "Define ON if running tests locally." OFF)

set(LIBLIFT_TEST_SOURCE_FILES
    setup.hpp
    test_async_request.cpp
    test_client.cpp
    test_debug_info.cpp
    test_escape.cpp
    test_header.cpp
    test_http.cpp
    test_mime_field.cpp
    test_proxy.cpp
    test_query_builder.cpp
    test_resolve_host.cpp
    test_share.cpp
    test_sync_request.cpp
    test_timesup.cpp
    test_transfer_progress_request.cpp
    test_user_data_request.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${LIBLIFT_TEST_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE lifthttp)

message("${PROJECT_NAME} LIFT_LOCALHOST_TESTS = ${LIFT_LOCALHOST_TESTS}")
if(LIFT_LOCALHOST_TESTS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LIFT_LOCALHOST_TESTS)
endif()

if(LIFT_CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)
endif()

add_test(NAME LiftHttpTest COMMAND ${PROJECT_NAME})
