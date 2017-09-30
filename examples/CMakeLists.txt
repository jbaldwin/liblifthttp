cmake_minimum_required(VERSION 3.3)
project(liblifthttp_examples)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(ASYNC_SIMPLE_SOURCE_FILES
    examples/async_simple.cpp
)
add_executable(async_simple ${ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(async_simple PRIVATE lifthttp)
target_link_libraries(async_simple PRIVATE uv curl)
