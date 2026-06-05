cmake_minimum_required(VERSION 3.20)

include(CMakeParseArguments)

project("file_to_c")
set(CMAKE_CXX_STANDARD 17)

add_executable(file_to_c "file_to_c.cpp")

find_package(ZSTD)

if(TARGET ZSTD::ZSTD)
    target_link_libraries(file_to_c PRIVATE ZSTD::ZSTD)
else()
    find_path(ZSTD_INCLUDE_DIR zstd.h)
    find_library(ZSTD_LIBRARY NAMES zstd libzstd libzstd_static)

    if(ZSTD_INCLUDE_DIR AND ZSTD_LIBRARY)
        target_include_directories(file_to_c PRIVATE ${ZSTD_INCLUDE_DIR})
        target_link_libraries(file_to_c PRIVATE ${ZSTD_LIBRARY})
    else()
        message(FATAL_ERROR "zstd library not found. Install zstd (e.g. via vcpkg) or ensure zstd.h and libzstd are on the include/library path.")
    endif()
endif()
