cmake_minimum_required(VERSION 3.3)
project(liblifthttp_examples)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

### async_simple ###
set(ASYNC_SIMPLE_SOURCE_FILES
    examples/async_simple.cpp
)
add_executable(async_simple ${ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(async_simple PRIVATE lifthttp uv curl)

### synch_simple ###
set(SYNCH_SIMPLE_SOURCE_FILES
    examples/synch_simple.cpp
)
add_executable(synch_simple ${SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(synch_simple PRIVATE lifthttp uv curl)

### query_builder ###
set(QUERY_BUILDER_SOURCE_FILES
    examples/query_builder.cpp
)
add_executable(query_builder ${QUERY_BUILDER_SOURCE_FILES})
target_link_libraries(query_builder PRIVATE lifthttp uv curl)

### request_headers ###
set(REQUEST_HEADERS_SIMPLE_SOURCE_FILES
    examples/request_headers_simple.cpp
)
add_executable(request_headers_simple ${REQUEST_HEADERS_SIMPLE_SOURCE_FILES})
target_link_libraries(request_headers_simple PRIVATE lifthttp uv curl)
