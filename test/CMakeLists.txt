cmake_minimum_required(VERSION 3.0)
project(liblifthttp_tests)

option(LIFT_LOCALHOST_TESTS "Define ON if running tests locally." OFF)

set(LIBLIFT_TEST_SOURCE_FILES
    AsyncRequestTest.hpp
    EscapeTest.hpp
    EventLoopTest.hpp
    HeaderTest.hpp
    HttpTest.hpp
    MimeFieldTest.hpp
    QueryBuilderTest.hpp
    ResolveHostTest.hpp
    SyncRequestTest.hpp
    TimesupTest.hpp
    TransferProgressRequest.hpp
    UserDataRequestTest.hpp
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
