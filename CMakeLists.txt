cmake_minimum_required(VERSION 3.3)
project(curlplus)
set(CMAKE_CXX_FLAGS "-std=c++14")

include_directories(inc)

set(SOURCE_FILES
    inc/AsyncRequest.h src/AsyncRequest.cpp
    inc/EventLoop.h src/EventLoop.cpp
    inc/Request.h src/Request.cpp
    src/main.cpp
)

add_executable(curlplus ${SOURCE_FILES})
target_link_libraries(curlplus uv curl)
