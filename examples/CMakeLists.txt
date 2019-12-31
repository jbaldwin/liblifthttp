cmake_minimum_required(VERSION 2.8)
project(liblifthttp_examples)

### synch_simple ###
set(LIBLIFTHTTP_SYNCH_SIMPLE_SOURCE_FILES
    synch_simple.cpp
)
add_executable(lift_synch_simple ${LIBLIFTHTTP_SYNCH_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_synch_simple PRIVATE lifthttp ${LIFT_LIB_DEPS})
target_compile_features(lift_synch_simple PRIVATE cxx_std_17)
target_include_directories(lift_synch_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### async_simple ###
set(LIBLIFTHTTP_ASYNC_SIMPLE_SOURCE_FILES
    async_simple.cpp
)
add_executable(lift_async_simple ${LIBLIFTHTTP_ASYNC_SIMPLE_SOURCE_FILES})
target_link_libraries(lift_async_simple PRIVATE lifthttp ${LIFT_LIB_DEPS})
target_compile_features(lift_async_simple PRIVATE cxx_std_17)
target_include_directories(lift_async_simple SYSTEM PRIVATE ${CURL_INCLUDE})

### benchmark ###
set(LIBLIFTHTTP_BENCHMARK_SOURCE_FILES
    benchmark.cpp
)
add_executable(lift_benchmark ${LIBLIFTHTTP_BENCHMARK_SOURCE_FILES})
target_link_libraries(lift_benchmark PRIVATE lifthttp ${LIFT_LIB_DEPS})
target_compile_features(lift_benchmark PRIVATE cxx_std_17)
target_include_directories(lift_benchmark SYSTEM PRIVATE ${CURL_INCLUDE})
