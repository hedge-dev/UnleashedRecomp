project("fshasher")

add_executable(fshasher "fshasher.cpp")

find_package(xxHash CONFIG REQUIRED)

target_link_libraries(fshasher PRIVATE xxHash::xxhash)
