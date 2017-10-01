cmake_minimum_required(VERSION 3.3)
project(lifthttp)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -Weffc++ -Werror -Wpedantic -pedantic-errors -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-vtables -MD -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-exit-time-destructors -Wno-global-constructors -Wno-error=deprecated-declarations -fcolor-diagnostics -ferror-limit=123")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -g -gdwarf-2 -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g -gdwarf-2")

set(SOURCE_FILES
    src/internal/CurlPool.h src/CurlPool.cpp
    inc/lift/Lift.h src/Lift.cpp
    inc/lift/EventLoop.h src/EventLoop.cpp
    inc/lift/Header.h src/Header.cpp
    inc/lift/IRequestCb.h
    inc/lift/Request.h src/Request.cpp
    inc/lift/RequestPool.h src/RequestPool.cpp
    inc/lift/RequestStatus.h
    inc/lift/Types.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC inc)
target_include_directories(${PROJECT_NAME} PRIVATE src/internal)

target_link_libraries(${PROJECT_NAME} PRIVATE
    uv
    curl
)

include(examples/CMakeLists.txt)
include(test/CMakeLists.txt)
