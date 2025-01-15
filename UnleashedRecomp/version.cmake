# version.cmake - written by hyperbx
# Generates a compilation unit from template files for version information.

# OUTPUT_FILE   : the original output file from a previous generation.
# TEMPLATE_FILE : the corresponding template file that was used to generate the output file.
function(CheckOutputFile OUTPUT_FILE TEMPLATE_FILE)
    if (NOT OUTPUT_FILE)
        message(FATAL_ERROR "OUTPUT_FILE not specified.")
    endif()

    if (NOT TEMPLATE_FILE)
        message(FATAL_ERROR "TEMPLATE_FILE not specified.")
    endif()

    if (EXISTS "${OUTPUT_FILE}")
        # Read original output file.
        file(READ "${OUTPUT_FILE}" ORIGINAL_CONTENT)

        # Read template file and configure.
        file(READ "${TEMPLATE_FILE}" TEMPLATE_CONTENT)
        string(CONFIGURE "${TEMPLATE_CONTENT}" TEMPLATE_FILE_FINAL_CONTENT @ONLY)

        # Check if configured template matches the original output file and replace it if not.
        if (NOT ORIGINAL_CONTENT STREQUAL TEMPLATE_FILE_FINAL_CONTENT)
            message("-- Creating ${OUTPUT_FILE}")
            file(WRITE "${OUTPUT_FILE}" "${TEMPLATE_FILE_FINAL_CONTENT}")
        endif()
    else()
        message("-- Creating ${OUTPUT_FILE}")
        configure_file("${TEMPLATE_FILE}" "${OUTPUT_FILE}" @ONLY)
    endif()
endfunction()

# OUTPUT_DIR                  : the output directory of the resulting *.h/*.cpp files.
# VERSION_TXT                 : the input text file containing the milestone, major, minor and revision variables.
# H_TEMPLATE                  : the input template for the header.
# CXX_TEMPLATE                : the input template for the source file.
# BUILD_TYPE                  : the current build configuration (e.g. "Release", "RelWithDebInfo", "Debug") (optional).
# IS_BUILD_TYPE_IN_VER_STRING : the build type should be appended to the version string (optional).
# IS_GIT_REPO                 : the project is part of a Git repository (optional).
function(GenerateVersionSources)
    cmake_parse_arguments(ARGS "" "OUTPUT_DIR;VERSION_TXT;H_TEMPLATE;CXX_TEMPLATE;BUILD_TYPE;IS_BUILD_TYPE_IN_VER_STRING;IS_GIT_REPO" "" ${ARGN})

    message("-- Generating version information...")

    if (NOT ARGS_OUTPUT_DIR)
        message(FATAL_ERROR "OUTPUT_DIR not specified.")
    endif()

    if (NOT ARGS_VERSION_TXT)
        message(FATAL_ERROR "VERSION_TXT not specified.")
    endif()

    if (NOT ARGS_H_TEMPLATE)
        message(FATAL_ERROR "H_TEMPLATE not specified.")
    endif()

    if (NOT ARGS_CXX_TEMPLATE)
        message(FATAL_ERROR "CXX_TEMPLATE not specified.")
    endif()

    set(BUILD_TYPE ${ARGS_BUILD_TYPE})

    if (ARGS_IS_GIT_REPO)
        find_package(Git REQUIRED)

        # Get Git branch name.
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE BRANCH_NAME
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # Get Git commit hash.
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
            OUTPUT_VARIABLE COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # Get short Git commit hash.
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            OUTPUT_VARIABLE COMMIT_HASH_SHORT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    file(READ "${ARGS_VERSION_TXT}" FILE_CONTENT)
    string(REGEX REPLACE "\r?\n" ";" FILE_LINES "${FILE_CONTENT}")

    foreach(LINE ${FILE_LINES})
        if (LINE STREQUAL "")
            continue()
        endif()

        # Find key/value pair match.
        string(REGEX MATCH "([A-Za-z_]+)=\\\"?([^\"]+)\\\"?" MATCH "${LINE}")

        if (MATCH)
            # Extract key/value pairs.
            string(REGEX REPLACE "([A-Za-z_]+)=.*" "\\1" KEY "${MATCH}")
            string(REGEX REPLACE "[A-Za-z_]+=\\\"?([^\"]+)\\\"?" "\\1" VALUE "${MATCH}")

            # Set environment variable.
            set("${KEY}" "${VALUE}")
        endif()
    endforeach()

    set(VERSION_STRING "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")

    # Prepend version milestone.
    if (VERSION_MILESTONE)
        string(PREPEND VERSION_STRING "${VERSION_MILESTONE} ")
    endif()
    
    # Append commit hash and branch.
    if (COMMIT_HASH_SHORT)
        string(APPEND VERSION_STRING ".${COMMIT_HASH_SHORT}-${BRANCH_NAME}")
    endif()

    # Append build configuration.
    if (ARGS_BUILD_TYPE AND ARGS_IS_BUILD_TYPE_IN_VER_STRING)
        string(APPEND VERSION_STRING " (${ARGS_BUILD_TYPE})")
    endif()

    message("-- Build: ${VERSION_STRING}")

    get_filename_component(H_TEMPLATE_NAME_WE "${ARGS_H_TEMPLATE}" NAME_WE)
    get_filename_component(CXX_TEMPLATE_NAME_WE "${ARGS_CXX_TEMPLATE}" NAME_WE)
    set(H_OUTPUT_FILE "${ARGS_OUTPUT_DIR}/${H_TEMPLATE_NAME_WE}.h")
    set(CXX_OUTPUT_FILE "${ARGS_OUTPUT_DIR}/${CXX_TEMPLATE_NAME_WE}.cpp")
    
    CheckOutputFile("${H_OUTPUT_FILE}" "${ARGS_H_TEMPLATE}")
    CheckOutputFile("${CXX_OUTPUT_FILE}" "${ARGS_CXX_TEMPLATE}")
endfunction()
