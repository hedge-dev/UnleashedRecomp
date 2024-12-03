cmake_minimum_required(VERSION 3.20)

include(CMakeParseArguments)

project("file_to_c")
set(CMAKE_CXX_STANDARD 17)

add_executable(file_to_c "file_to_c.cpp")

function(BIN2C)
    cmake_parse_arguments(BIN2C_ARGS "" "SOURCE_FILE;DEST_FILE;ARRAY_TYPE;ARRAY_NAME" "" ${ARGN})

    if(NOT BIN2C_ARGS_SOURCE_FILE)
        message(FATAL_ERROR "SOURCE_FILE not specified.")
    endif()

    if(NOT BIN2C_ARGS_DEST_FILE)
        set(BIN2C_ARGS_DEST_FILE "${BIN2C_ARGS_SOURCE_FILE}")
    endif()

    add_custom_command(OUTPUT "${BIN2C_ARGS_DEST_FILE}.c"
        COMMAND file_to_c "${BIN2C_ARGS_SOURCE_FILE}" "${BIN2C_ARGS_ARRAY_NAME}" "${BIN2C_ARGS_ARRAY_TYPE}" "${BIN2C_ARGS_DEST_FILE}.c" "${BIN2C_ARGS_DEST_FILE}.h"
        DEPENDS "${BIN2C_ARGS_SOURCE_FILE}" file_to_c
        BYPRODUCTS "${BIN2C_ARGS_DEST_FILE}.h"
        COMMENT "Generating binary header for ${BIN2C_ARGS_SOURCE_FILE}..."
    )
endfunction()
