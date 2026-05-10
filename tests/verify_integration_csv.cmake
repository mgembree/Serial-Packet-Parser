if(NOT DEFINED CSV_PATH)
    message(FATAL_ERROR "CSV_PATH was not provided")
endif()

get_filename_component(_csv_name "${CSV_PATH}" NAME)

set(_config "")
if(DEFINED CTEST_CONFIGURATION_TYPE)
    set(_config "${CTEST_CONFIGURATION_TYPE}")
elseif(DEFINED ENV{CTEST_CONFIGURATION_TYPE})
    set(_config "$ENV{CTEST_CONFIGURATION_TYPE}")
endif()

if(NOT DEFINED BINARY_DIR)
    set(BINARY_DIR "")
endif()

message(STATUS "verify_integration_csv: CSV_PATH='${CSV_PATH}'")
message(STATUS "verify_integration_csv: CMAKE_CURRENT_BINARY_DIR='${CMAKE_CURRENT_BINARY_DIR}'")
message(STATUS "verify_integration_csv: CMAKE_CURRENT_LIST_DIR='${CMAKE_CURRENT_LIST_DIR}'")
message(STATUS "verify_integration_csv: CTEST_CONFIGURATION_TYPE='${_config}'")

set(_candidate_paths "${CSV_PATH}")
if(NOT "${_config}" STREQUAL "")
    list(APPEND _candidate_paths
        "${CMAKE_CURRENT_BINARY_DIR}/${_config}/${_csv_name}"
        "${BINARY_DIR}/${_config}/${_csv_name}"
    )
endif()

list(APPEND _candidate_paths
    "${CMAKE_CURRENT_BINARY_DIR}/${_csv_name}"
    "${BINARY_DIR}/${_csv_name}"
    "${CMAKE_CURRENT_BINARY_DIR}/Debug/${_csv_name}"
    "${CMAKE_CURRENT_BINARY_DIR}/Release/${_csv_name}"
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/${_csv_name}"
    "${CMAKE_CURRENT_BINARY_DIR}/MinSizeRel/${_csv_name}"
)

set(_resolved_csv_path "")
foreach(_candidate IN LISTS _candidate_paths)
    if(NOT "${_candidate}" STREQUAL "" AND EXISTS "${_candidate}")
        set(_resolved_csv_path "${_candidate}")
        break()
    endif()
endforeach()

if("${_resolved_csv_path}" STREQUAL "")
    string(REPLACE ";" "\n  - " _formatted_candidates "${_candidate_paths}")
    message(FATAL_ERROR
        "Expected CSV output was not created. Tried paths:\n"
        "  - ${_formatted_candidates}"
    )
endif()

message(STATUS "verify_integration_csv: using CSV='${_resolved_csv_path}'")

file(READ "${_resolved_csv_path}" csv_text)
string(REPLACE "\r\n" "\n" csv_text "${csv_text}")
string(REPLACE "\r" "\n" csv_text "${csv_text}")

set(expected_header_current "timestamp,type,temperature_c,voltage_v,status_flags,payload_hex")
set(expected_header_legacy "timestamp_iso8601,packet_type,temperature_c,voltage_v,status_flags,payload_hex")
string(FIND "${csv_text}" "${expected_header_current}" header_pos_current)
string(FIND "${csv_text}" "${expected_header_legacy}" header_pos_legacy)
if(header_pos_current EQUAL -1 AND header_pos_legacy EQUAL -1)
    message(FATAL_ERROR
        "CSV header mismatch in ${_resolved_csv_path}. Expected one of:\n"
        "  - ${expected_header_current}\n"
        "  - ${expected_header_legacy}"
    )
endif()

string(REGEX MATCHALL "\n" newline_matches "${csv_text}")
list(LENGTH newline_matches newline_count)
if(newline_count LESS 4)
    message(FATAL_ERROR "Expected at least 3 data rows plus header in ${_resolved_csv_path}")
endif()

string(FIND "${csv_text}" ",temperature," temp_pos)
string(FIND "${csv_text}" ",voltage," voltage_pos)
string(FIND "${csv_text}" ",status," status_pos)
if(temp_pos EQUAL -1 OR voltage_pos EQUAL -1 OR status_pos EQUAL -1)
    message(FATAL_ERROR "Expected decoded packet types temperature/voltage/status were not all present")
endif()
