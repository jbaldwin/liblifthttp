cmake_minimum_required(VERSION 3.3)
project(curlplus)
set(CMAKE_CXX_FLAGS "-std=c++14")

include_directories(inc)

set(SOURCE_FILES
        inc/EventLoop.h src/EventLoop.cpp
    src/main.cpp
)

add_executable(curlplus ${SOURCE_FILES})
