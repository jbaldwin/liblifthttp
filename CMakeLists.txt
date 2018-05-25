cmake_minimum_required(VERSION 2.8)
project(lifthttp CXX)

message(${CMAKE_CXX_COMPILER_ID})

# Prefer the environment variable over the -D parameters
if(NOT "$ENV{CURL_INCLUDE}" STREQUAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CURL_INCLUDE}")
endif()

if(NOT "$ENV{LIBSSL}" STREQUAL "")
    set(LIBSSL $ENV{LIBSSL})
else()
    set(LIBSSL "" CACHE STRING "User specified libsss static location.")
endif()

if(NOT "$ENV{LIBCRYPTO}" STREQUAL "")
    set(LIBCRYPTO $ENV{LIBCRYPTO})
else()
    set(LIBCRYPTO "" CACHE STRING "User specificed libcrypto static location.")
endif()

if(NOT "$ENV{LIBCURL}" STREQUAL "")
    set(LIBCURL $ENV{LIBCURL})
else()
    set(LIBCURL "curl" CACHE STRING "User specified libcurl static location.")
endif()

if(NOT "$ENV{LIBCARES}" STREQUAL "")
    set(LIBCARES $ENV{LIBCARES})
else()
    set(LIBCARES "" CACHE STRING "User specified libcares static location.")
endif()

message("LIBSSL = $ENV{LIBSSL}")
message("LIBCRYPTO = $ENV{LIBCRYPTO}")
message("LIBCURL = $ENV{LIBCURL}")
message("LIBCARES = $ENV{LIBCARES}")

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/lib)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-vtables -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-exit-time-destructors -Wno-global-constructors -Wno-error=deprecated-declarations -Wno-disabled-macro-expansion -Wno-covered-switch-default -fcolor-diagnostics -ferror-limit=128")
else()
    message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER}")
endif()

set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wextra -Weffc++ -Werror -Wpedantic -pedantic-errors -MD ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -gdwarf-2 -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -g -gdwarf-2")

message("CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES
    inc/lift/CurlPool.h src/CurlPool.cpp
    inc/lift/Escape.h src/Escape.cpp
    inc/lift/EventLoop.h inc/lift/EventLoop.tcc src/EventLoop.cpp
    inc/lift/Header.h src/Header.cpp
    inc/lift/Lift.h src/Lift.cpp
    inc/lift/Http.h
    inc/lift/QueryBuilder.h src/QueryBuilder.cpp
    inc/lift/Request.h src/Request.cpp
    inc/lift/RequestHandle.h src/RequestHandle.cpp
    inc/lift/RequestPool.h src/RequestPool.cpp
    inc/lift/RequestStatus.h src/RequestStatus.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -Wl,--start-group ${LIBSSL} ${LIBCRYPTO} ${LIBCURL} ${LIBCARES} -Wl,--end-group
    uv
    pthread
    z
    dl
)

include(examples/CMakeLists.txt)
