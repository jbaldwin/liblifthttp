cmake_minimum_required(VERSION 3.0)
project(liblifthttp_tests)

set(LIBLIFT_TEST_SOURCE_FILES
    AsyncRequestTest.hpp
    EscapeTest.hpp
    QueryBuilderTest.hpp
    SyncRequestTest.hpp
    TimesupTest.hpp
    TransferProgressRequest.hpp
    UserDataRequestTest.hpp
)

add_executable(${PROJECT_NAME} main.cpp ${LIBLIFT_TEST_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE lifthttp)

if(LIFT_CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)
endif()

add_test(NAME LiftHttpTest COMMAND ${PROJECT_NAME})
