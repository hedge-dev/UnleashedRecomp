project("xcontent_extract")

add_executable(xcontent_extract
    "xcontent_extract.cpp"
    "${CMAKE_SOURCE_DIR}/UnleashedRecomp/install/xcontent_file_system.cpp"
)

target_include_directories(xcontent_extract PRIVATE
    "${CMAKE_SOURCE_DIR}/UnleashedRecomp/install"
    "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils"
)

target_link_libraries(xcontent_extract PRIVATE XenonUtils)
