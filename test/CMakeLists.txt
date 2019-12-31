cmake_minimum_required(VERSION 2.8)
project(liblifthttp_tests)

set(LIBLIFT_TEST_SOURCE_FILES
    AsyncRequestTest.hpp
    EscapeTest.hpp
    QueryBuilderTest.hpp
    SyncRequestTest.hpp
    TransferProgressRequest.hpp
    UserDataRequestTest.hpp
)

add_executable(${PROJECT_NAME} main.cpp ${LIBLIFT_TEST_SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIFT_CURL_LIBRARY_DEPENDENCIES}
    ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES}
    lifthttp
)
add_test(NAME LiftHttpTest COMMAND ${PROJECT_NAME})