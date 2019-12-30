cmake_minimum_required(VERSION 2.8)
project(liblifthttp_examples)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)

### synch_simple ###
set(SYNCH_SIMPLE_SOURCE_FILES
    synch_simple.cpp
)
add_executable(lift_synch_simple ${SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_synch_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_synch_simple PRIVATE cxx_std_17)
target_include_directories(lift_synch_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple ###
set(ASYNC_SIMPLE_SOURCE_FILES
    async_simple.cpp
)
add_executable(lift_async_simple ${ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_async_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_simple PRIVATE cxx_std_17)
target_include_directories(lift_async_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple_max_bytes ###
set(ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES
    async_simple_max_bytes.cpp
)
add_executable(lift_async_simple_max_bytes ${ASYNC_SIMPLE_MAX_BYTES_SOURCE_FILES})
target_link_libraries(lift_async_simple_max_bytes PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_simple_max_bytes PRIVATE cxx_std_17)
target_include_directories(lift_async_simple_max_bytes SYSTEM PRIVATE ${CURL_INCLUDE})

### async_batch_requests ###
set(ASYNC_BATCH_REQUESTS_SOURCE_FILES
    async_batch_requests.cpp
)
add_executable(lift_async_batch_requests ${ASYNC_BATCH_REQUESTS_SOURCE_FILES})
target_link_libraries(lift_async_batch_requests PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_batch_requests PRIVATE cxx_std_17)
target_include_directories(lift_async_batch_requests SYSTEM PRIVATE ${CURL_INCLUDE})


### async_post_requests ###
set(ASYNC_POST_REQUESTS_SOURCE_FILES
    async_post_requests.cpp
)
add_executable(lift_async_post_requests ${ASYNC_POST_REQUESTS_SOURCE_FILES})
target_link_libraries(lift_async_post_requests PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_post_requests PRIVATE cxx_std_17)
target_include_directories(lift_async_post_requests SYSTEM PRIVATE ${CURL_INCLUDE})

### query_builder ###
set(QUERY_BUILDER_SOURCE_FILES
    query_builder.cpp
)
add_executable(lift_query_builder ${QUERY_BUILDER_SOURCE_FILES})
target_link_libraries(lift_query_builder PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_query_builder PRIVATE cxx_std_17)
target_include_directories(lift_query_builder SYSTEM PRIVATE ${CURL_INCLUDE})

### request_headers ###
set(REQUEST_HEADERS_SIMPLE_SOURCE_FILES
    request_headers_simple.cpp
)
add_executable(lift_request_headers_simple ${REQUEST_HEADERS_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_request_headers_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_request_headers_simple PRIVATE cxx_std_17)
target_include_directories(lift_request_headers_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### benchmark_simple ###
set(BENCHMARK_SIMPLE_SOURCE_FILES
    benchmark_simple.cpp
)
add_executable(lift_benchmark_simple ${BENCHMARK_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_benchmark_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_benchmark_simple PRIVATE cxx_std_17)
target_include_directories(lift_benchmark_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### user_data ###
set(USER_DATA_SOURCE_FILES
    user_data.cpp
)
add_executable(lift_user_data ${USER_DATA_SOURCE_FILES})
target_link_libraries(lift_user_data PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_user_data PRIVATE cxx_std_17)
target_include_directories(lift_user_data SYSTEM PRIVATE ${CURL_INCLUDE})


### transfer_progress_simple ###
set(TRANSFER_PROGRESS_SIMPLE_SOURCE_FILES
    transfer_progress_simple.cpp
)
add_executable(lift_transfer_progress_simple ${TRANSFER_PROGRESS_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_transfer_progress_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_transfer_progress_simple PRIVATE cxx_std_17)
target_include_directories(lift_transfer_progress_simple SYSTEM PRIVATE ${CURL_INCLUDE})
