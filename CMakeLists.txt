cmake_minimum_required(VERSION 3.16)
project(serial_packet_parser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build parser tests" ON)

add_library(spp_core
    src/checksum.cpp
    src/parser.cpp
    src/decoder.cpp
    src/logger.cpp
)

target_include_directories(spp_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(spp_core PUBLIC cxx_std_17)

target_compile_options(spp_core PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

add_executable(serial_packet_parser src/main.cpp)
target_link_libraries(serial_packet_parser PRIVATE spp_core)

if(BUILD_TESTS)
    enable_testing()
    add_executable(spp_tests tests/test_parser.cpp)
    target_link_libraries(spp_tests PRIVATE spp_core)
    add_test(NAME spp_tests COMMAND spp_tests)
endif()
