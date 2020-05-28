cmake_minimum_required(VERSION 3.0)
project(lifthttp CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIFT_BUILD_EXAMPLES "Build the examples. Default=ON" ON)
option(LIFT_BUILD_TESTS    "Build the tests. Default=ON" ON)
option(LIFT_CODE_COVERAGE  "Enable code coverage, tests must also be enabled. Default=OFF" OFF)

if(NOT DEFINED LIFT_USER_LINK_LIBRARIES)
    set(LIFT_USER_LINK_LIBRARIES curl CACHE STRING "User specified additional link targets for custom curl and ssl libraries, defaults to system 'curl'.")
endif()

set(LIFT_LINK_LIBRARIES
    uv
    pthread
    z
    dl
    stdc++fs
    ${LIFT_USER_LINK_LIBRARIES}
)

message("${PROJECT_NAME} LIFT_BUILD_EXAMPLES = ${LIFT_BUILD_EXAMPLES}")
message("${PROJECT_NAME} LIFT_BUILD_TESTS    = ${LIFT_BUILD_TESTS}")
message("${PROJECT_NAME} LIFT_CODE_COVERAGE  = ${LIFT_CODE_COVERAGE}")
message("${PROJECT_NAME} LIFT_LINK_LIBRARIES = ${LIFT_LINK_LIBRARIES}")

set(LIBLIFT_SOURCE_FILES
    inc/lift/Const.hpp
    inc/lift/Escape.hpp src/Escape.cpp
    inc/lift/EventLoop.hpp src/EventLoop.cpp
    inc/lift/Executor.hpp src/Executor.cpp
    inc/lift/Header.hpp src/Header.cpp
    inc/lift/Lift.hpp src/Lift.cpp
    inc/lift/LiftStatus.hpp src/LiftStatus.cpp
    inc/lift/MimeField.hpp src/MimeField.cpp
    inc/lift/Http.hpp src/Http.cpp
    inc/lift/QueryBuilder.hpp src/QueryBuilder.cpp
    inc/lift/Request.hpp src/Request.cpp
    inc/lift/ResolveHost.hpp src/ResolveHost.cpp
    inc/lift/Response.hpp src/Response.cpp
)

add_library(${PROJECT_NAME} STATIC ${LIBLIFT_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LIFT_CURL_INCLUDE})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIFT_LINK_LIBRARIES})

if(LIFT_CODE_COVERAGE)
    target_compile_options(${PROJECT_NAME} PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE
        -Wno-unknown-pragmas
    )
endif()

if(LIFT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LIFT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
