project("fshasher")

add_executable(fshasher "fshasher.cpp")

target_link_libraries(fshasher PRIVATE xxHash::xxhash)
