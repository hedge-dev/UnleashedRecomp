project("UnleashedRecomp")
set(TARGET_NAME "SWA")

add_compile_options(
    /fp:strict
    -march=sandybridge
    -fno-strict-aliasing

    -Wno-switch
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-void-pointer-to-int-cast
    -Wno-int-to-void-pointer-cast
    -Wno-invalid-offsetof
)

add_compile_definitions(
    SWA_IMPL
    SDL_MAIN_HANDLED
    _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR # Microsoft wtf?
    _CRT_SECURE_NO_WARNINGS)

# Generate icon bitmap header for SDL surface.
BIN2H(SOURCE_FILE "res/icon.bmp" HEADER_FILE "res/icon.h" ARRAY_TYPE "unsigned char" VARIABLE_NAME "g_icon")
BIN2H(SOURCE_FILE "res/icon_night.bmp" HEADER_FILE "res/icon_night.h" ARRAY_TYPE "unsigned char" VARIABLE_NAME "g_iconNight")

set(SWA_PRECOMPILED_HEADERS
    "stdafx.h"
)

set(SWA_CFG_CXX_SOURCES
    "cfg/config.cpp"
    "cfg/config_detail.cpp"
)

set(SWA_KERNEL_CXX_SOURCES
    "kernel/imports.cpp"
    "kernel/xdm.cpp"
    "kernel/heap.cpp"
    "kernel/memory.cpp"
    "kernel/xam.cpp"
    "kernel/io/file_system.cpp"
)

set(SWA_CPU_CXX_SOURCES
    "cpu/guest_thread.cpp"
    "cpu/code_cache.cpp"
)

set(SWA_GPU_CXX_SOURCES
    "gpu/video.cpp"
    "gpu/imgui_snapshot.cpp"
    "gpu/rhi/rt64_d3d12.cpp"
    "gpu/rhi/rt64_vulkan.cpp"
)

set(SWA_APU_CXX_SOURCES
    "apu/audio.cpp"
    "apu/driver/xaudio_driver.cpp"
)

set(SWA_HID_CXX_SOURCES
    "hid/hid.cpp"
    "hid/driver/sdl_hid.cpp"
)

set(SWA_PATCHES_CXX_SOURCES
    "patches/ui/frontend_listener.cpp"
    "patches/fps_patches.cpp"
    "patches/misc_patches.cpp"
    "patches/object_patches.cpp"
    "patches/player_patches.cpp"
    "patches/resident_patches.cpp"
    "patches/video_patches.cpp"
)

set(SWA_UI_CXX_SOURCES
    "ui/window.cpp"
)

set(SWA_INSTALL_CXX_SOURCES
    "install/installer.cpp"
    "install/iso_file_system.cpp"
    "install/memory_mapped_file.cpp"
    "install/xcontent_file_system.cpp"
    "install/xex_patcher.cpp"
    "install/hashes/apotos_shamar.cpp"
    "install/hashes/chunnan.cpp"
    "install/hashes/empire_city_adabat.cpp"
    "install/hashes/game.cpp"
    "install/hashes/holoska.cpp"
    "install/hashes/mazuri.cpp"
    "install/hashes/spagonia.cpp"
    "install/hashes/update.cpp"
)

set(LIBMSPACK_PATH ${SWA_THIRDPARTY_ROOT}/libmspack/libmspack/mspack)

set(LIBMSPACK_C_SOURCES
    ${LIBMSPACK_PATH}/lzxd.c
)

set_source_files_properties(${LIBMSPACK_C_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

set(SMOLV_SOURCE_DIR "${SWA_THIRDPARTY_ROOT}/ShaderRecomp/thirdparty/smol-v/source")

set(SWA_CXX_SOURCES
    "app.cpp"
    "main.cpp"
    "misc_impl.cpp"
    "stdafx.cpp"

    ${SWA_CFG_CXX_SOURCES}
    ${SWA_KERNEL_CXX_SOURCES}
    ${SWA_CPU_CXX_SOURCES}
    ${SWA_GPU_CXX_SOURCES}
    ${SWA_APU_CXX_SOURCES}
    ${SWA_HID_CXX_SOURCES}
    ${SWA_PATCHES_CXX_SOURCES}
    ${SWA_UI_CXX_SOURCES}
    ${SWA_INSTALL_CXX_SOURCES}
    ${LIBMSPACK_C_SOURCES}
    "${SMOLV_SOURCE_DIR}/smolv.cpp"
)

if (WIN32)
    # Set up Win32 resources for application icon.
    set(ICON_PATH "${PROJECT_SOURCE_DIR}/res/win32/icon.ico")
    configure_file("res/win32/res.rc" "${CMAKE_BINARY_DIR}/res.rc" @ONLY)
    add_executable(UnleashedRecomp ${SWA_CXX_SOURCES} "${CMAKE_BINARY_DIR}/res.rc")
else()
    add_executable(UnleashedRecomp ${SWA_CXX_SOURCES})
endif()

set_target_properties(UnleashedRecomp PROPERTIES OUTPUT_NAME ${TARGET_NAME})

find_package(directx-headers CONFIG REQUIRED)
find_package(directx12-agility CONFIG REQUIRED)
find_package(d3d12-memory-allocator CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(unordered_dense CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(volk CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(tomlplusplus REQUIRED IMPORTED_TARGET tomlplusplus)
find_package(directx-dxc REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-concurrentqueue REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(unofficial-tiny-aes-c CONFIG REQUIRED)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/D3D12)
add_custom_command(TARGET UnleashedRecomp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> ${CMAKE_CURRENT_BINARY_DIR}/D3D12
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> ${CMAKE_CURRENT_BINARY_DIR}/D3D12
   COMMAND_EXPAND_LISTS
)

file(COPY ${PACKAGE_PREFIX_DIR}/bin/dxil.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(UnleashedRecomp PRIVATE
    Microsoft::DirectX-Headers 
    Microsoft::DirectX-Guids 
    Microsoft::DirectX12-Agility
    Microsoft::DirectXShaderCompiler
    comctl32
    dxgi
    Vulkan::Headers
    volk::volk 
    volk::volk_headers
    GPUOpen::VulkanMemoryAllocator
    ntdll
    o1heap
    PowerUtils
    SDL2::SDL2-static
    UnleashedRecompLib
    unofficial::D3D12MemoryAllocator
    unordered_dense::unordered_dense
    winmm
    xxHash::xxhash
    PkgConfig::tomlplusplus
    zstd::libzstd_static
    unofficial::concurrentqueue::concurrentqueue
    Synchronization
    imgui::imgui
    magic_enum::magic_enum
    unofficial::tiny-aes-c::tiny-aes-c
)

target_include_directories(UnleashedRecomp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${SWA_THIRDPARTY_ROOT}/ddspp
    ${SWA_THIRDPARTY_ROOT}/TinySHA1
    ${LIBMSPACK_PATH}
    ${Stb_INCLUDE_DIR}
    ${SMOLV_SOURCE_DIR}
)

target_precompile_headers(UnleashedRecomp PUBLIC ${SWA_PRECOMPILED_HEADERS})

function(compile_shader FILE_PATH TARGET_NAME)
    set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gpu/shader/${FILE_PATH}.hlsl)
    cmake_path(GET FILE_PATH STEM VARIABLE_NAME)
    add_custom_command(
        OUTPUT ${FILE_PATH}.dxil.h
        COMMAND ${DIRECTX_DXC_TOOL} -T ${TARGET_NAME} -HV 2021 -all-resources-bound -Wno-ignored-attributes -Fh ${FILE_PATH}.dxil.h ${FILE_PATH} -Vn g_${VARIABLE_NAME}_dxil
        DEPENDS ${FILE_PATH}
    )
    add_custom_command(
        OUTPUT ${FILE_PATH}.spirv.h
        COMMAND ${DIRECTX_DXC_TOOL} -T ${TARGET_NAME} -HV 2021 -all-resources-bound -spirv -fvk-use-dx-layout ${ARGN} -Fh ${FILE_PATH}.spirv.h ${FILE_PATH} -Vn g_${VARIABLE_NAME}_spirv
        DEPENDS ${FILE_PATH}
    )
    target_sources(UnleashedRecomp PRIVATE ${FILE_PATH}.dxil.h ${FILE_PATH}.spirv.h)
endfunction()

function(compile_vertex_shader FILE_PATH)
    compile_shader(${FILE_PATH} vs_6_0 -fvk-invert-y)
endfunction()

function(compile_pixel_shader FILE_PATH)
    compile_shader(${FILE_PATH} ps_6_0)
endfunction()

compile_vertex_shader(copy_vs)
compile_pixel_shader(gamma_correction_ps)
compile_pixel_shader(imgui_ps)
compile_vertex_shader(imgui_vs)
compile_pixel_shader(movie_ps)
compile_vertex_shader(movie_vs)
compile_pixel_shader(resolve_msaa_depth_2x)
compile_pixel_shader(resolve_msaa_depth_4x)
compile_pixel_shader(resolve_msaa_depth_8x)

function(generate_aggregate_header INPUT_DIRECTORY OUTPUT_FILE)
    get_filename_component(ABS_OUTPUT_FILE "${OUTPUT_FILE}" ABSOLUTE)
    file(GLOB_RECURSE HEADER_FILES "${INPUT_DIRECTORY}/*.h")
    set(HEADER_CONTENT "#pragma once\n\n")

    foreach(HEADER_FILE IN LISTS HEADER_FILES)
        get_filename_component(ABS_HEADER_FILE "${HEADER_FILE}" ABSOLUTE)
        if (ABS_HEADER_FILE STREQUAL ABS_OUTPUT_FILE)
            continue()
        endif()
        file(RELATIVE_PATH RELATIVE_HEADER_FILE "${INPUT_DIRECTORY}" "${HEADER_FILE}")
        string(APPEND HEADER_CONTENT "#include \"${RELATIVE_HEADER_FILE}\"\n")
    endforeach()

    if (EXISTS "${OUTPUT_FILE}")
        file(READ "${OUTPUT_FILE}" EXISTING_CONTENT)
        if (EXISTING_CONTENT STREQUAL HEADER_CONTENT)
            return()
        endif()
    endif()

    file(WRITE "${OUTPUT_FILE}" "${HEADER_CONTENT}")
endfunction()

generate_aggregate_header(
    "${CMAKE_CURRENT_SOURCE_DIR}/api"
    "${CMAKE_CURRENT_SOURCE_DIR}/api/SWA.h"
)
