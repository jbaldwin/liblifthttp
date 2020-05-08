cmake_minimum_required(VERSION 2.8)
project(lifthttp CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED LIFT_CURL_INCLUDE)
    set(LIFT_CURL_INCLUDE "" CACHE PATH "User specified libcurl include directory.")
endif()
if(NOT DEFINED LIBSSL)
    set(LIFT_LIBSSL "" CACHE FILEPATH "User specified libssl static location.")
endif()
if(NOT DEFINED LIFT_LIBCRYPTO)
    set(LIFT_LIBCRYPTO "" CACHE FILEPATH "User specificed libcrypto static location.")
endif()
if(NOT DEFINED LIFT_LIBCURL)
    set(LIFT_LIBCURL "-lcurl" CACHE FILEPATH "User specified libcurl static location.")
endif()
if(NOT DEFINED LIFT_LIBCARES)
    set(LIFT_LIBCARES "" CACHE FILEPATH "User specified libcares static location.")
endif()

message("${PROJECT_NAME} LIFT_CURL_INCLUDE = ${LIFT_CURL_INCLUDE}")
message("${PROJECT_NAME} LIFT_LIBSSL       = ${LIFT_LIBSSL}")
message("${PROJECT_NAME} LIFT_LIBCRYPTO    = ${LIFT_LIBCRYPTO}")
message("${PROJECT_NAME} LIFT_LIBCURL      = ${LIFT_LIBCURL}")
message("${PROJECT_NAME} LIFT_LIBCARES     = ${LIFT_LIBCARES}")

set(LIFT_SYSTEM_LIBRARY_DEPENDENCIES
    uv
    pthread
    z
    dl
    stdc++fs
)

set(LIFT_CURL_LIBRARY_DEPENDENCIES
    "-Wl,--start-group ${LIFT_LIBSSL} ${LIFT_LIBCRYPTO} ${LIFT_LIBCURL} ${LIFT_LIBCARES} -Wl,--end-group"
)

set(LIFT_LIB_DEPS "${LIFT_SYSTEM_LIBRARY_DEPENDENCIES} ${LIFT_CURL_LIBRARY_DEPENDENCIES}")

set(LIBLIFT_SOURCE_FILES
    inc/lift/Const.hpp
    inc/lift/Escape.hpp src/Escape.cpp
    inc/lift/EventLoop.hpp src/EventLoop.cpp
    inc/lift/Executor.hpp src/Executor.cpp
    inc/lift/HeaderView.hpp src/HeaderView.cpp
    inc/lift/Lift.hpp src/Lift.cpp
    inc/lift/LiftStatus.hpp src/LiftStatus.cpp
    inc/lift/MimeField.hpp src/MimeField.cpp
    inc/lift/Http.hpp src/Http.cpp
    inc/lift/QueryBuilder.hpp src/QueryBuilder.cpp
    inc/lift/Request.hpp src/Request.cpp
    inc/lift/ResolveHost.hpp src/ResolveHost.cpp
    inc/lift/Response.hpp src/Response.cpp
)

add_library(${PROJECT_NAME} STATIC SHARED ${LIBLIFT_SOURCE_FILES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LIFT_CURL_INCLUDE})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIFT_LIB_DEPS})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Wno-unknown-pragmas
    )
endif()

add_subdirectory(examples)
enable_testing()
add_subdirectory(test)
