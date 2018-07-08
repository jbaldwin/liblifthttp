cmake_minimum_required(VERSION 2.8)
project(liblifthttp_examples)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)

### synch_simple ###
set(SYNCH_SIMPLE_SOURCE_FILES
    examples/synch_simple.cpp
)
add_executable(synch_simple ${SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(synch_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### async_simple ###
set(ASYNC_SIMPLE_SOURCE_FILES
    examples/async_simple.cpp
)
add_executable(async_simple ${ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(async_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### async_simple_max_bytes ###
set(ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES
    examples/async_simple_max_bytes.cpp
)
add_executable(async_simple_max_bytes ${ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES})
target_link_libraries(async_simple_max_bytes PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### async_batch_requests ###
set(ASYNC_BATCH_REQUESTS_SOURCE_FILES
        examples/async_batch_requests.cpp
)
add_executable(async_batch_requests ${ASYNC_BATCH_REQUESTS_SOURCE_FILES})
target_link_libraries(async_batch_requests PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### query_builder ###
set(QUERY_BUILDER_SOURCE_FILES
    examples/query_builder.cpp
)
add_executable(query_builder ${QUERY_BUILDER_SOURCE_FILES})
target_link_libraries(query_builder PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### request_headers ###
set(REQUEST_HEADERS_SIMPLE_SOURCE_FILES
    examples/request_headers_simple.cpp
)
add_executable(request_headers_simple ${REQUEST_HEADERS_SIMPLE_SOURCE_FILES})
target_link_libraries(request_headers_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### benchmark_simple ###
set(BENCHMARK_SIMPLE_SOURCE_FILES
    examples/benchmark_simple.cpp
)
add_executable(benchmark_simple ${BENCHMARK_SIMPLE_SOURCE_FILES})
target_link_libraries(benchmark_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})

### user_data ###
set(USER_DATA_SOURCE_FILES
    examples/user_data.cpp
)
add_executable(user_data ${USER_DATA_SOURCE_FILES})
target_link_libraries(user_data PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
