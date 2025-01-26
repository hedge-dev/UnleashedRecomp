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

# VERSION_TXT : the input text file containing the milestone, major, minor and revision variables.
function(ParseVersionInfo VERSION_TXT)
    if (NOT VERSION_TXT)
        message(FATAL_ERROR "VERSION_TXT not specified.")
    endif()
    
    if (NOT EXISTS "${VERSION_TXT}")
        message(FATAL_ERROR "No such file: ${VERSION_TXT}")
    endif()

    file(READ "${VERSION_TXT}" FILE_CONTENT)
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
            set("${KEY}" "${VALUE}" CACHE INTERNAL "${KEY}")
        endif()
    endforeach()
endfunction()

# VERSION_TXT     : the input text file containing the milestone, major, minor and revision variables.
# OUTPUT_CSV      : the output string will be formatted as #,#,# without any alphabetic characters (optional).
# BUILD_TYPE      : the current build configuration (e.g. "Release", "RelWithDebInfo", "Debug") (optional).
# SHOW_GIT_INFO   : the Git commit hash and branch name should be appended to the version string (optional).
# SHOW_BUILD_TYPE : the build type should be appended to the version string (optional).
# OUTPUT_VAR      : the output variable to store the version string in.
function(CreateVersionString)
    cmake_parse_arguments(ARGS "" "VERSION_TXT;OUTPUT_CSV;BUILD_TYPE;SHOW_GIT_INFO;SHOW_BUILD_TYPE;OUTPUT_VAR" "" ${ARGN})
    
    if (NOT ARGS_VERSION_TXT)
        message(FATAL_ERROR "VERSION_TXT not specified.")
    endif()

    if (NOT ARGS_OUTPUT_VAR)
        message(FATAL_ERROR "OUTPUT_VAR not specified.")
    endif()

    ParseVersionInfo("${ARGS_VERSION_TXT}")

    if (ARGS_OUTPUT_CSV)
        set(VERSION_STRING "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_REVISION}")
    else()
        set(VERSION_STRING "v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}")

        if (VERSION_MILESTONE)
            string(PREPEND VERSION_STRING "${VERSION_MILESTONE} ")
        endif()

        if (ARGS_SHOW_GIT_INFO)
            find_package(Git REQUIRED)

            # Get Git branch name.
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
                OUTPUT_VARIABLE BRANCH_NAME
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            set(BRANCH_NAME ${BRANCH_NAME} CACHE INTERNAL "BRANCH_NAME")

            # Get Git commit hash.
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
                OUTPUT_VARIABLE COMMIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            set(COMMIT_HASH ${COMMIT_HASH} CACHE INTERNAL "COMMIT_HASH")

            # Get short Git commit hash.
            execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                OUTPUT_VARIABLE COMMIT_HASH_SHORT
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )

            set(COMMIT_HASH_SHORT ${COMMIT_HASH_SHORT} CACHE INTERNAL "COMMIT_HASH_SHORT")
    
            # Append commit hash and branch.
            if (COMMIT_HASH_SHORT AND BRANCH_NAME)
                string(APPEND VERSION_STRING ".${COMMIT_HASH_SHORT}-${BRANCH_NAME}")
            endif()
        endif()

        # Append build configuration.
        if (ARGS_BUILD_TYPE AND ARGS_SHOW_BUILD_TYPE)
            string(APPEND VERSION_STRING " (${ARGS_BUILD_TYPE})")
        endif()
    endif()

    if (ARGS_OUTPUT_VAR)
        set(${ARGS_OUTPUT_VAR} ${VERSION_STRING} PARENT_SCOPE)
    endif()
endfunction()

# OUTPUT_DIR      : the output directory of the resulting *.h/*.cpp files.
# VERSION_TXT     : the input text file containing the milestone, major, minor and revision variables.
# H_TEMPLATE      : the input template for the header.
# CXX_TEMPLATE    : the input template for the source file.
# BUILD_TYPE      : the current build configuration (e.g. "Release", "RelWithDebInfo", "Debug") (optional).
# SHOW_GIT_INFO   : the Git commit hash and branch name should be appended to the version string (optional).
# SHOW_BUILD_TYPE : the build type should be appended to the version string (optional).
function(GenerateVersionSources)
    cmake_parse_arguments(ARGS "" "OUTPUT_DIR;VERSION_TXT;H_TEMPLATE;CXX_TEMPLATE;BUILD_TYPE;SHOW_GIT_INFO;SHOW_BUILD_TYPE" "" ${ARGN})

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

    # Required for *.cpp template.
    set(BUILD_TYPE ${ARGS_BUILD_TYPE})

    CreateVersionString(
        VERSION_TXT ${ARGS_VERSION_TXT}
        BUILD_TYPE ${ARGS_BUILD_TYPE}
        SHOW_GIT_INFO ${ARGS_SHOW_GIT_INFO}
        SHOW_BUILD_TYPE ${ARGS_SHOW_BUILD_TYPE}
        OUTPUT_VAR VERSION_STRING
    )

    message("-- Build: ${VERSION_STRING}")

    get_filename_component(H_TEMPLATE_NAME_WE "${ARGS_H_TEMPLATE}" NAME_WE)
    get_filename_component(CXX_TEMPLATE_NAME_WE "${ARGS_CXX_TEMPLATE}" NAME_WE)
    set(H_OUTPUT_FILE "${ARGS_OUTPUT_DIR}/${H_TEMPLATE_NAME_WE}.h")
    set(CXX_OUTPUT_FILE "${ARGS_OUTPUT_DIR}/${CXX_TEMPLATE_NAME_WE}.cpp")
    
    CheckOutputFile("${H_OUTPUT_FILE}" "${ARGS_H_TEMPLATE}")
    CheckOutputFile("${CXX_OUTPUT_FILE}" "${ARGS_CXX_TEMPLATE}")
endfunction()
