cmake_minimum_required(VERSION 2.8)
project(lifthttp CXX)

# Create a Debug/lib or Release/lib output directory.
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/lib)

# Custom locations for libcurl dependencies.
if(NOT "$ENV{CURL_INCLUDE}" STREQUAL "")
    set(LIFT_CURL_INCLUDE "$ENV{CURL_INCLUDE}")
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

message("LIBSSL = ${LIBSSL}")
message("LIBCRYPTO = ${LIBCRYPTO}")
message("LIBCURL = ${LIBCURL}")
message("LIBCARES = ${LIBCARES}")

set(LIFT_SYSTEM_LIBRARY_DEPENDENCIES
    uv
    pthread
    z
    dl
)

set(LIFT_CURL_LIBRARY_DEPENDENCIES
    -Wl,--start-group ${LIBSSL} ${LIBCRYPTO} ${LIBCURL} ${LIBCARES} -Wl,--end-group
)

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
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LIFT_CURL_LIBRARY_DEPENDENCIES}
    ${LIFT_SYSTEM_LIBRARY_DEPENDENCIES}
)

target_compile_options(
    ${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Weffc++
    -Werror
    -Wpedantic
    -pedantic-errors
)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Wno-unknown-pragmas
    )
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-padded
        -Wno-weak-vtables
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-error=deprecated-declarations
        -Wno-disabled-macro-expansion
        -Wno-covered-switch-default
        -fcolor-diagnostics
        -ferror-limit=128
    )
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -O0
        -g
        -gdwarf-2
        -D_GLIBCXX_DEBUG
    )
else() # "Release"
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -O2
        -g
        -gdwarf-2
    )
endif()

include(examples/CMakeLists.txt)
