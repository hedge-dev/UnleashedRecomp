project("UnleashedRecomp")
set(TARGET_NAME "SWA")

option(SWA_XAUDIO2 "Use XAudio2 for audio playback" OFF)

function(BIN2C)
    cmake_parse_arguments(BIN2C_ARGS "" "TARGET_OBJ;SOURCE_FILE;DEST_FILE;ARRAY_NAME;COMPRESSION_TYPE" "" ${ARGN})

    if(NOT BIN2C_ARGS_TARGET_OBJ)
        message(FATAL_ERROR "TARGET_OBJ not specified.")
    endif()

    if(NOT BIN2C_ARGS_SOURCE_FILE)
        message(FATAL_ERROR "SOURCE_FILE not specified.")
    endif()

    if(NOT BIN2C_ARGS_DEST_FILE)
        set(BIN2C_ARGS_DEST_FILE "${BIN2C_ARGS_SOURCE_FILE}")
    endif()

    if(NOT BIN2C_ARGS_COMPRESSION_TYPE)
        set(BIN2C_ARGS_COMPRESSION_TYPE "none")
    endif()

    add_custom_command(OUTPUT "${BIN2C_ARGS_DEST_FILE}.c"
        COMMAND file_to_c "${BIN2C_ARGS_SOURCE_FILE}" "${BIN2C_ARGS_ARRAY_NAME}" "${BIN2C_ARGS_COMPRESSION_TYPE}" "${BIN2C_ARGS_DEST_FILE}.c" "${BIN2C_ARGS_DEST_FILE}.h"
        DEPENDS "${BIN2C_ARGS_SOURCE_FILE}" file_to_c
        BYPRODUCTS "${BIN2C_ARGS_DEST_FILE}.h"
        COMMENT "Generating binary header for ${BIN2C_ARGS_SOURCE_FILE}..."
    )

    set_source_files_properties(${BIN2C_ARGS_DEST_FILE}.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    target_sources(${BIN2C_ARGS_TARGET_OBJ} PRIVATE ${BIN2C_ARGS_DEST_FILE}.c)
endfunction()

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

set(SWA_PRECOMPILED_HEADERS
    "stdafx.h"
)

set(SWA_KERNEL_CXX_SOURCES
    "kernel/imports.cpp"
    "kernel/xdm.cpp"
    "kernel/heap.cpp"
    "kernel/memory.cpp"
    "kernel/xam.cpp"
    "kernel/io/file_system.cpp"
)

set(SWA_OS_CXX_SOURCES
    "os/media.cpp"
    "os/process.cpp"
    "os/version.cpp"
)

if (WIN32)
    list(APPEND SWA_OS_CXX_SOURCES
        "os/win32/media_win32.cpp"
        "os/win32/process_win32.cpp"
        "os/win32/version_win32.cpp"
    )
endif()

set(SWA_CPU_CXX_SOURCES
    "cpu/guest_thread.cpp"
    "cpu/code_cache.cpp"
)

set(SWA_GPU_CXX_SOURCES
    "gpu/video.cpp"
    "gpu/imgui/imgui_common.cpp"
    "gpu/imgui/imgui_font_builder.cpp"
    "gpu/imgui/imgui_snapshot.cpp"
    "gpu/rhi/plume_d3d12.cpp"
    "gpu/rhi/plume_vulkan.cpp"
)

set(SWA_APU_CXX_SOURCES
    "apu/audio.cpp"
    "apu/embedded_player.cpp"
)

if (SWA_XAUDIO2)
    list(APPEND SWA_APU_CXX_SOURCES "apu/driver/xaudio_driver.cpp")
else()
    list(APPEND SWA_APU_CXX_SOURCES "apu/driver/miniaudio_driver.cpp")
endif()

set(SWA_HID_CXX_SOURCES
    "hid/hid.cpp"
    "hid/driver/sdl_hid.cpp"
)

set(SWA_PATCHES_CXX_SOURCES
    "patches/ui/CHudPause_patches.cpp"
    "patches/ui/CTitleStateIntro_patches.cpp"
    "patches/ui/CTitleStateMenu_patches.cpp"
    "patches/ui/frontend_listener.cpp"
    "patches/audio_patches.cpp"
    "patches/camera_patches.cpp"
    "patches/fps_patches.cpp"
    "patches/misc_patches.cpp"
    "patches/object_patches.cpp"
    "patches/player_patches.cpp"
    "patches/resident_patches.cpp"
    "patches/video_patches.cpp"
)

set(SWA_UI_CXX_SOURCES
    "ui/achievement_menu.cpp"
    "ui/achievement_overlay.cpp"
    "ui/installer_wizard.cpp"
    "ui/button_guide.cpp"
    "ui/fader.cpp"
    "ui/message_window.cpp"
    "ui/options_menu.cpp"
    "ui/sdl_listener.cpp"
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
set(BC_DIFF_SOURCE_DIR "${SWA_TOOLS_ROOT}/bc_diff")

set(MINIAUDIO_INCLUDE_DIRS "${SWA_THIRDPARTY_ROOT}/miniaudio")

set(SWA_USER_CXX_SOURCES
    "user/achievement_data.cpp"
    "user/config.cpp"
    "user/config_detail.cpp"
)

set(SWA_CXX_SOURCES
    "app.cpp"
    "exports.cpp"
    "main.cpp"
    "misc_impl.cpp"
    "stdafx.cpp"
    
    ${SWA_KERNEL_CXX_SOURCES}
    ${SWA_OS_CXX_SOURCES}
    ${SWA_CPU_CXX_SOURCES}
    ${SWA_GPU_CXX_SOURCES}
    ${SWA_APU_CXX_SOURCES}
    ${SWA_HID_CXX_SOURCES}
    ${SWA_PATCHES_CXX_SOURCES}
    ${SWA_UI_CXX_SOURCES}
    ${SWA_INSTALL_CXX_SOURCES}
    ${LIBMSPACK_C_SOURCES}
    "${SMOLV_SOURCE_DIR}/smolv.cpp"
    ${SWA_USER_CXX_SOURCES}
)

if (WIN32)
    # Set up Win32 resources for application icon.
    set(ICON_PATH "${PROJECT_SOURCE_DIR}/../UnleashedRecompResources/images/game_icon.ico")
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
find_package(nfd CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

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
    nfd::nfd
    msdf-atlas-gen::msdf-atlas-gen
    Vorbis::vorbisfile
)

target_include_directories(UnleashedRecomp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${SWA_THIRDPARTY_ROOT}/ddspp
    ${SWA_THIRDPARTY_ROOT}/TinySHA1
    ${LIBMSPACK_PATH}
    ${Stb_INCLUDE_DIR}
    ${SMOLV_SOURCE_DIR}
    ${MINIAUDIO_INCLUDE_DIRS}
    ${BC_DIFF_SOURCE_DIR}
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
compile_pixel_shader(csd_filter_ps)
compile_pixel_shader(enhanced_motion_blur_ps)
compile_pixel_shader(gaussian_blur_3x3)
compile_pixel_shader(gaussian_blur_5x5)
compile_pixel_shader(gaussian_blur_7x7)
compile_pixel_shader(gaussian_blur_9x9)
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

set(RESOURCES_SOURCE_PATH "${PROJECT_SOURCE_DIR}/../UnleashedRecompResources")
set(RESOURCES_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/res")

BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/bc_diff/button_bc_diff.bin" DEST_FILE "${RESOURCES_OUTPUT_PATH}/bc_diff/button_bc_diff.bin" ARRAY_NAME "g_button_bc_diff" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/font/im_font_atlas.bin" DEST_FILE "${RESOURCES_OUTPUT_PATH}/font/im_font_atlas.bin" ARRAY_NAME "g_im_font_atlas" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/font/im_font_atlas.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/font/im_font_atlas.dds" ARRAY_NAME "g_im_font_atlas_texture" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/achievements_menu/trophy.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/achievements_menu/trophy.dds" ARRAY_NAME "g_trophy" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/controller.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/controller.dds" ARRAY_NAME "g_controller" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/general_window.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/general_window.dds" ARRAY_NAME "g_general_window" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/kbm.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/kbm.dds" ARRAY_NAME "g_kbm" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/select_fade.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/select_fade.dds" ARRAY_NAME "g_select_fade" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/select_fill.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/select_fill.dds" ARRAY_NAME "g_select_fill" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/arrow_circle.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/arrow_circle.dds" ARRAY_NAME "g_arrow_circle" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_001.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_001.dds" ARRAY_NAME "g_install_001" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_002.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_002.dds" ARRAY_NAME "g_install_002" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_003.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_003.dds" ARRAY_NAME "g_install_003" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_004.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_004.dds" ARRAY_NAME "g_install_004" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_005.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_005.dds" ARRAY_NAME "g_install_005" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_006.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_006.dds" ARRAY_NAME "g_install_006" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_007.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_007.dds" ARRAY_NAME "g_install_007" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/install_008.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/install_008.dds" ARRAY_NAME "g_install_008" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/miles_electric_icon.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/miles_electric_icon.dds" ARRAY_NAME "g_miles_electric_icon" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/installer/pulse_install.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/installer/pulse_install.dds" ARRAY_NAME "g_pulse_install" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/achievement_notifications.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/achievement_notifications.dds" ARRAY_NAME "g_achievement_notifications" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/allow_background_input.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/allow_background_input.dds" ARRAY_NAME "g_allow_background_input" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/antialiasing.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/antialiasing.dds" ARRAY_NAME "g_antialiasing" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/aspect_ratio.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/aspect_ratio.dds" ARRAY_NAME "g_aspect_ratio" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/battle_theme.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/battle_theme.dds" ARRAY_NAME "g_battle_theme" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/brightness.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/brightness.dds" ARRAY_NAME "g_brightness" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/control_tutorial.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/control_tutorial.dds" ARRAY_NAME "g_control_tutorial" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/controller_icons.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/controller_icons.dds" ARRAY_NAME "g_controller_icons" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/effects_volume.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/effects_volume.dds" ARRAY_NAME "g_effects_volume" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/fps.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/fps.dds" ARRAY_NAME "g_fps" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/fullscreen.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/fullscreen.dds" ARRAY_NAME "g_fullscreen" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/gi_texture_filtering.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/gi_texture_filtering.dds" ARRAY_NAME "g_gi_texture_filtering" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/hints.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/hints.dds" ARRAY_NAME "g_hints" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/invert_camera_x.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/invert_camera_x.dds" ARRAY_NAME "g_invert_camera_x" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/invert_camera_y.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/invert_camera_y.dds" ARRAY_NAME "g_invert_camera_y" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/language.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/language.dds" ARRAY_NAME "g_language" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/monitor.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/monitor.dds" ARRAY_NAME "g_monitor" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/motion_blur.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/motion_blur.dds" ARRAY_NAME "g_motion_blur" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/movie_scale_mode.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/movie_scale_mode.dds" ARRAY_NAME "g_movie_scale_mode" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/music_attenuation.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/music_attenuation.dds" ARRAY_NAME "g_music_attenuation" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/music_volume.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/music_volume.dds" ARRAY_NAME "g_music_volume" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/resolution_scale.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/resolution_scale.dds" ARRAY_NAME "g_resolution_scale" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution.dds" ARRAY_NAME "g_shadow_resolution" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/subtitles.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/subtitles.dds" ARRAY_NAME "g_subtitles" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/time_of_day_transition.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/time_of_day_transition.dds" ARRAY_NAME "g_time_of_day_transition" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/transparency_antialiasing.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/transparency_antialiasing.dds" ARRAY_NAME "g_transparency_antialiasing" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/ui_scale_mode.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/ui_scale_mode.dds" ARRAY_NAME "g_ui_scale_mode" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/voice_language.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/voice_language.dds" ARRAY_NAME "g_voice_language" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/vsync.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/vsync.dds" ARRAY_NAME "g_vsync" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/window_size.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/window_size.dds" ARRAY_NAME "g_window_size" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/xbox_color_correction.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/xbox_color_correction.dds" ARRAY_NAME "g_xbox_color_correction" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/game_icon.bmp" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/game_icon.bmp" ARRAY_NAME "g_game_icon")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/game_icon_night.bmp" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/game_icon_night.bmp" ARRAY_NAME "g_game_icon_night")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_worldmap_cursor.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_worldmap_cursor.ogg" ARRAY_NAME "g_sys_worldmap_cursor")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_worldmap_finaldecide.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_worldmap_finaldecide.ogg" ARRAY_NAME "g_sys_worldmap_finaldecide")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausecansel.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausecansel.ogg" ARRAY_NAME "g_sys_actstg_pausecansel")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausecursor.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausecursor.ogg" ARRAY_NAME "g_sys_actstg_pausecursor")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausedecide.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausedecide.ogg" ARRAY_NAME "g_sys_actstg_pausedecide")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausewinclose.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausewinclose.ogg" ARRAY_NAME "g_sys_actstg_pausewinclose")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausewinopen.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausewinopen.ogg" ARRAY_NAME "g_sys_actstg_pausewinopen")

