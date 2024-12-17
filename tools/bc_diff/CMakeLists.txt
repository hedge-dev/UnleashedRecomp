cmake_minimum_required(VERSION 3.20)

project("bc_diff")
set(CMAKE_CXX_STANDARD 17)

add_executable(bc_diff "bc_diff.cpp")
add_compile_definitions(bc_diff PRIVATE _CRT_SECURE_NO_WARNINGS)

target_link_libraries(bc_diff PRIVATE xxHash::xxhash)
