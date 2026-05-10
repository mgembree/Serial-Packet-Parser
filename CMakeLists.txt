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
    src/hex_input.cpp
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

    # Use a config-specific output path for multi-config generators (VS/Xcode).
    set(SPP_INTEGRATION_CSV_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    set(SPP_INTEGRATION_CSV "${SPP_INTEGRATION_CSV_DIR}/spp_integration_output.csv")
    add_test(
        NAME spp_cli_integration_run
        COMMAND serial_packet_parser
            --input "${CMAKE_CURRENT_SOURCE_DIR}/data/sample_stream.bin"
            --output "${SPP_INTEGRATION_CSV}"
            --input-format binary
            --csv-flush-interval 1
    )
    set_tests_properties(
        spp_cli_integration_run
        PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            FIXTURES_SETUP spp_cli_csv
    )

    add_test(
        NAME spp_cli_integration_verify
        COMMAND ${CMAKE_COMMAND}
            -DCSV_PATH=${SPP_INTEGRATION_CSV}
            -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_integration_csv.cmake
    )
    set_tests_properties(
        spp_cli_integration_verify
        PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            FIXTURES_REQUIRED spp_cli_csv
    )
endif()
