cmake_minimum_required(VERSION 3.5)
project(lifthttp CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIFT_BUILD_EXAMPLES "Build the examples. Default=ON" ON)
option(LIFT_BUILD_TESTS    "Build the tests. Default=ON" ON)
option(LIFT_CODE_COVERAGE  "Enable code coverage, tests must also be enabled. Default=OFF" OFF)
option(LIFT_RUN_GITCONFIG  "Set the githooks directory to auto format code and update the readme, Default=OFF." OFF)

find_package(CURL REQUIRED)
find_package(libuv CONFIG QUIET)
find_package(Threads REQUIRED)

set(LIBUV_TARGET "")
# LibUV >=1.45 introduced an imported target 'libuv::uv'
if(TARGET libuv::uv)
    set(LIBUV_TARGET libuv::uv)
else()
    find_package(PkgConfig REQUIRED)
    # Fallback to pkg-config if libuv CMake config is not found
    pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
    set(LIBUV_TARGET PkgConfig::LIBUV)
endif()

message("${PROJECT_NAME} LIFT_BUILD_EXAMPLES      = ${LIFT_BUILD_EXAMPLES}")
message("${PROJECT_NAME} LIFT_BUILD_TESTS         = ${LIFT_BUILD_TESTS}")
message("${PROJECT_NAME} LIFT_CODE_COVERAGE       = ${LIFT_CODE_COVERAGE}")
message("${PROJECT_NAME} LIFT_RUN_GITCONFIG       = ${LIFT_RUN_GITCONFIG}")

if (LIFT_RUN_GITCONFIG)
    # Set the githooks directory to auto format and update the readme.
    message("${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR} -> git config --local core.hooksPath .githooks")
    execute_process(
        COMMAND git config --local core.hooksPath .githooks
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

if(MSVC)
    # curl/curl.h on Windows introduces the 'max' macro definition causing std::numeric_limits<uint8_t>::max compilation to fail
    add_definitions(-DNOMINMAX)
endif()

# LibUV >=1.45 introduced an imported target 'libuv::uv'
set(LIBUV_TARGET $<IF:$<TARGET_EXISTS:libuv::uv>,libuv::uv,uv>)

set(LIBLIFTHTTP_SOURCE_FILES
    inc/lift/impl/copy_util.hpp
    inc/lift/impl/pragma.hpp

    inc/lift/client_pool.hpp src/client_pool.cpp
    inc/lift/client.hpp src/client.cpp
    inc/lift/const.hpp
    inc/lift/escape.hpp src/escape.cpp
    inc/lift/executor.hpp src/executor.cpp
    inc/lift/header.hpp src/header.cpp
    inc/lift/http.hpp src/http.cpp
    inc/lift/init.hpp src/init.cpp
    inc/lift/lift_status.hpp src/lift_status.cpp
    inc/lift/lift.hpp
    inc/lift/mime_field.hpp src/mime_field.cpp
    inc/lift/query_builder.hpp src/query_builder.cpp
    inc/lift/request.hpp src/request.cpp
    inc/lift/resolve_host.hpp src/resolve_host.cpp
    inc/lift/response.hpp src/response.cpp
)

add_library(${PROJECT_NAME} ${LIBLIFTHTTP_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LIFT_CURL_INCLUDE})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl ${LIBUV_TARGET} Threads::Threads)

if(LIFT_CODE_COVERAGE)
    if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
        target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-update=atomic)
    endif()
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Weffc++
        # -Werror
    )
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Weffc++
        #-Werror
        -Weverything
        -Wpedantic
        # -pedantic-errors
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-padded
        -Wno-switch-enum
        -Wno-covered-switch-default
        -Wno-disabled-macro-expansion
    )
endif()

if(LIFT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LIFT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
)