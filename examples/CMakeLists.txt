cmake_minimum_required(VERSION 2.8)
project(liblifthttp_examples)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)

### synch_simple ###
set(SYNCH_SIMPLE_SOURCE_FILES
    examples/synch_simple.cpp
)
add_executable(synch_simple ${SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(synch_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(synch_simple PRIVATE cxx_std_17)
target_include_directories(synch_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple ###
set(ASYNC_SIMPLE_SOURCE_FILES
    examples/async_simple.cpp
)
add_executable(async_simple ${ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(async_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(async_simple PRIVATE cxx_std_17)
target_include_directories(async_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple_max_bytes ###
set(ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES
    examples/async_simple_max_bytes.cpp
)
add_executable(async_simple_max_bytes ${ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES})
target_link_libraries(async_simple_max_bytes PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(async_simple_max_bytes PRIVATE cxx_std_17)
target_include_directories(async_simple_max_bytes SYSTEM PRIVATE ${CURL_INCLUDE})

### async_batch_requests ###
set(ASYNC_BATCH_REQUESTS_SOURCE_FILES
        examples/async_batch_requests.cpp
)
add_executable(async_batch_requests ${ASYNC_BATCH_REQUESTS_SOURCE_FILES})
target_link_libraries(async_batch_requests PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(async_batch_requests PRIVATE cxx_std_17)
target_include_directories(async_batch_requests SYSTEM PRIVATE ${CURL_INCLUDE})

### query_builder ###
set(QUERY_BUILDER_SOURCE_FILES
    examples/query_builder.cpp
)
add_executable(query_builder ${QUERY_BUILDER_SOURCE_FILES})
target_link_libraries(query_builder PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(query_builder PRIVATE cxx_std_17)
target_include_directories(query_builder SYSTEM PRIVATE ${CURL_INCLUDE})

### request_headers ###
set(REQUEST_HEADERS_SIMPLE_SOURCE_FILES
    examples/request_headers_simple.cpp
)
add_executable(request_headers_simple ${REQUEST_HEADERS_SIMPLE_SOURCE_FILES})
target_link_libraries(request_headers_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(request_headers_simple PRIVATE cxx_std_17)
target_include_directories(request_headers_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### benchmark_simple ###
set(BENCHMARK_SIMPLE_SOURCE_FILES
    examples/benchmark_simple.cpp
)
add_executable(benchmark_simple ${BENCHMARK_SIMPLE_SOURCE_FILES})
target_link_libraries(benchmark_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(benchmark_simple PRIVATE cxx_std_17)
target_include_directories(benchmark_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### user_data ###
set(USER_DATA_SOURCE_FILES
    examples/user_data.cpp
)
add_executable(user_data ${USER_DATA_SOURCE_FILES})
target_link_libraries(user_data PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(user_data PRIVATE cxx_std_17)
target_include_directories(user_data SYSTEM PRIVATE ${CURL_INCLUDE})
