cmake_minimum_required(VERSION 2.8)
project(liblifthttp_tests)

add_executable(${PROJECT_NAME} main.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIFT_CURL_LIBRARY_DEPENDENCIES}
    ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES}
    lifthttp
)
add_test(NAME LiftHttpTest COMMAND ${PROJECT_NAME})