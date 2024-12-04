cmake_minimum_required(VERSION 3.20)

include(CMakeParseArguments)

project("file_to_c")
set(CMAKE_CXX_STANDARD 17)

add_executable(file_to_c "file_to_c.cpp")

find_package(zstd CONFIG REQUIRED)
target_link_libraries(file_to_c PRIVATE $<IF:$<TARGET_EXISTS:zstd::libzstd_static>,zstd::libzstd_static,zstd::libzstd>)
