project("fshasher")

add_executable(fshasher "fshasher.cpp")

find_package(xxhash CONFIG REQUIRED)

target_link_libraries(fshasher PRIVATE xxHash::xxhash)
