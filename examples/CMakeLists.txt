cmake_minimum_required(VERSION 2.8)
project(liblifthttp_examples)

### synch_simple ###
set(SYNCH_SIMPLE_SOURCE_FILES
    synch_simple.cpp
)
add_executable(lift_synch_simple ${SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_synch_simple PRIVATE lifthttp ${LIFT_LIB_DEPS})
target_compile_features(lift_synch_simple PRIVATE cxx_std_17)
target_include_directories(lift_synch_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple ###
set(LIBLIFTHTTP_ASYNC_SIMPLE_SOURCE_FILES
    async_simple.cpp
)
add_executable(lift_async_simple ${LIBLIFTHTTP_ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_async_simple PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_simple PRIVATE cxx_std_17)
target_include_directories(lift_async_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_post_requests ###
set(ASYNC_POST_REQUESTS_SOURCE_FILES
    async_post_requests.cpp
)
add_executable(lift_async_post_requests ${ASYNC_POST_REQUESTS_SOURCE_FILES})
target_link_libraries(lift_async_post_requests PRIVATE lifthttp ${LIFT_CURL_LIBRARY_DEPENDENCIES} ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES})
target_compile_features(lift_async_post_requests PRIVATE cxx_std_17)
target_include_directories(lift_async_post_requests SYSTEM PRIVATE ${CURL_INCLUDE})

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
