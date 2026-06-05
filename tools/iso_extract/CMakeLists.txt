project("iso_extract")

add_executable(iso_extract
    "iso_extract.cpp"
    "${CMAKE_SOURCE_DIR}/UnleashedRecomp/install/iso_file_system.cpp"
)

target_include_directories(iso_extract PRIVATE
    "${CMAKE_SOURCE_DIR}/UnleashedRecomp/install"
    "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils"
)

target_link_libraries(iso_extract PRIVATE XenonUtils)
