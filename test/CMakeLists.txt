cmake_minimum_required(VERSION 2.8)
project(lifthttp_test)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(TEST_SOURCE_FILES
    test/main.cpp
)

enable_testing()
add_executable(${PROJECT_NAME} ${TEST_SOURCE_FILES})
add_test(${PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} PRIVATE lifthttp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -Wl,-\( ${LIBSSL} ${LIBCRYPTO} ${LIBCURL} ${LIBCARES} -Wl,-\)
    uv
    pthread
    z
    dl
)
