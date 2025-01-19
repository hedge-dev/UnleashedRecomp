project("UnleashedRecomp")
set(TARGET_NAME "SWA")

if (WIN32)
    option(SWA_D3D12 "Add D3D12 support for rendering" ON)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    option(SWA_FLATPAK "Configure the build for Flatpak compatibility." OFF)
endif()

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
        COMMAND $<TARGET_FILE:file_to_c> "${BIN2C_ARGS_SOURCE_FILE}" "${BIN2C_ARGS_ARRAY_NAME}" "${BIN2C_ARGS_COMPRESSION_TYPE}" "${BIN2C_ARGS_DEST_FILE}.c" "${BIN2C_ARGS_DEST_FILE}.h"
        DEPENDS "${BIN2C_ARGS_SOURCE_FILE}"
        BYPRODUCTS "${BIN2C_ARGS_DEST_FILE}.h"
        COMMENT "Generating binary header for ${BIN2C_ARGS_SOURCE_FILE}..."
    )

    set_source_files_properties(${BIN2C_ARGS_DEST_FILE}.c PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    target_sources(${BIN2C_ARGS_TARGET_OBJ} PRIVATE ${BIN2C_ARGS_DEST_FILE}.c)
endfunction()

add_compile_options(
    -march=sandybridge
    -fno-strict-aliasing

    -Wno-switch
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-void-pointer-to-int-cast
    -Wno-int-to-void-pointer-cast
    -Wno-invalid-offsetof
    -Wno-null-arithmetic
    -Wno-null-conversion
    -Wno-tautological-undefined-compare
)

if (WIN32)
    add_compile_options(/fp:strict)
else()
    add_compile_options(-ffp-model=strict)
endif()

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

set(SWA_LOCALE_CXX_SOURCES
    "locale/config_locale.cpp"
    "locale/locale.cpp"
)

set(SWA_OS_CXX_SOURCES
    "os/logger.cpp"
    "os/media.cpp"
    "os/process.cpp"
    "os/user.cpp"
    "os/version.cpp"
)

if (WIN32)
    list(APPEND SWA_OS_CXX_SOURCES
        "os/win32/logger_win32.cpp"
        "os/win32/media_win32.cpp"
        "os/win32/process_win32.cpp"
        "os/win32/user_win32.cpp"
        "os/win32/version_win32.cpp"
    )
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
    list(APPEND SWA_OS_CXX_SOURCES
        "os/linux/logger_linux.cpp"
        "os/linux/media_linux.cpp"
        "os/linux/process_linux.cpp"
        "os/linux/user_linux.cpp"
        "os/linux/version_linux.cpp"
    )
endif()

set(SWA_CPU_CXX_SOURCES
    "cpu/guest_thread.cpp"
)

set(SWA_GPU_CXX_SOURCES
    "gpu/video.cpp"
    "gpu/imgui/imgui_common.cpp"
    "gpu/imgui/imgui_font_builder.cpp"
    "gpu/imgui/imgui_snapshot.cpp"
    "gpu/rhi/plume_vulkan.cpp"
)

if (SWA_D3D12)
    list(APPEND SWA_GPU_CXX_SOURCES
        "gpu/rhi/plume_d3d12.cpp"
    )
endif()

set(SWA_APU_CXX_SOURCES
    "apu/audio.cpp"
    "apu/embedded_player.cpp"
)

if (SWA_XAUDIO2)
    list(APPEND SWA_APU_CXX_SOURCES "apu/driver/xaudio_driver.cpp")
else()
    list(APPEND SWA_APU_CXX_SOURCES "apu/driver/sdl2_driver.cpp")
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
    "patches/aspect_ratio_patches.cpp"
    "patches/audio_patches.cpp"
    "patches/camera_patches.cpp"
    "patches/CGameModeStageTitle_patches.cpp"
    "patches/fps_patches.cpp"
    "patches/input_patches.cpp"
    "patches/inspire_patches.cpp"
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
    "ui/options_menu_thumbnails.cpp"
    "ui/options_menu.cpp"
    "ui/sdl_listener.cpp"
    "ui/game_window.cpp"
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

set(SWA_USER_CXX_SOURCES
    "user/achievement_data.cpp"
    "user/config.cpp"
)

set(SWA_MOD_CXX_SOURCES
    "mod/mod_loader.cpp"
)

set(SWA_THIRDPARTY_SOURCES
    "${SWA_THIRDPARTY_ROOT}/imgui/backends/imgui_impl_sdl2.cpp"
    "${SWA_THIRDPARTY_ROOT}/imgui/imgui.cpp"
    "${SWA_THIRDPARTY_ROOT}/imgui/imgui_demo.cpp"
    "${SWA_THIRDPARTY_ROOT}/imgui/imgui_draw.cpp"
    "${SWA_THIRDPARTY_ROOT}/imgui/imgui_tables.cpp"
    "${SWA_THIRDPARTY_ROOT}/imgui/imgui_widgets.cpp"
    "${SWA_THIRDPARTY_ROOT}/implot/implot.cpp"
    "${SWA_THIRDPARTY_ROOT}/implot/implot_demo.cpp"
    "${SWA_THIRDPARTY_ROOT}/implot/implot_items.cpp"
    "${SWA_THIRDPARTY_ROOT}/libmspack/libmspack/mspack/lzxd.c"
    "${SWA_THIRDPARTY_ROOT}/tiny-AES-c/aes.c"
    "${SWA_TOOLS_ROOT}/ShaderRecomp/thirdparty/smol-v/source/smolv.cpp"
)

set(SWA_THIRDPARTY_INCLUDES
    "${SWA_THIRDPARTY_ROOT}/concurrentqueue"
    "${SWA_THIRDPARTY_ROOT}/ddspp"
    "${SWA_THIRDPARTY_ROOT}/imgui"
    "${SWA_THIRDPARTY_ROOT}/implot"
    "${SWA_THIRDPARTY_ROOT}/libmspack/libmspack/mspack"
    "${SWA_THIRDPARTY_ROOT}/magic_enum/include"
    "${SWA_THIRDPARTY_ROOT}/stb"
    "${SWA_THIRDPARTY_ROOT}/tiny-AES-c"
    "${SWA_THIRDPARTY_ROOT}/TinySHA1"
    "${SWA_THIRDPARTY_ROOT}/unordered_dense/include"
    "${SWA_THIRDPARTY_ROOT}/volk"
    "${SWA_THIRDPARTY_ROOT}/Vulkan-Headers/include"
    "${SWA_THIRDPARTY_ROOT}/VulkanMemoryAllocator/include"
    "${SWA_TOOLS_ROOT}/bc_diff"
    "${SWA_TOOLS_ROOT}/ShaderRecomp/thirdparty/smol-v/source"
)

if (SWA_D3D12)
    list(APPEND SWA_THIRDPARTY_INCLUDES "${SWA_THIRDPARTY_ROOT}/D3D12MemoryAllocator/include")
    list(APPEND SWA_THIRDPARTY_SOURCES "${SWA_THIRDPARTY_ROOT}/D3D12MemoryAllocator/src/D3D12MemAlloc.cpp")
endif()

set_source_files_properties(${SWA_THIRDPARTY_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

set(SWA_CXX_SOURCES
    "app.cpp"
    "exports.cpp"
    "main.cpp"
    "misc_impl.cpp"
    "stdafx.cpp"
    "version.cpp"
    
    ${SWA_KERNEL_CXX_SOURCES}
    ${SWA_LOCALE_CXX_SOURCES}
    ${SWA_OS_CXX_SOURCES}
    ${SWA_CPU_CXX_SOURCES}
    ${SWA_GPU_CXX_SOURCES}
    ${SWA_APU_CXX_SOURCES}
    ${SWA_HID_CXX_SOURCES}
    ${SWA_PATCHES_CXX_SOURCES}
    ${SWA_UI_CXX_SOURCES}
    ${SWA_INSTALL_CXX_SOURCES}
    ${SWA_USER_CXX_SOURCES}
    ${SWA_MOD_CXX_SOURCES}
    ${SWA_THIRDPARTY_SOURCES}
)

if (WIN32)
    # Set up Win32 resources for application icon.
    set(ICON_PATH "${PROJECT_SOURCE_DIR}/../UnleashedRecompResources/images/game_icon.ico")
    configure_file("res/win32/res.rc.template" "${CMAKE_BINARY_DIR}/res.rc" @ONLY)
    add_executable(UnleashedRecomp ${SWA_CXX_SOURCES} "${CMAKE_BINARY_DIR}/res.rc")
else()
    add_executable(UnleashedRecomp ${SWA_CXX_SOURCES})
endif()

set_target_properties(UnleashedRecomp PROPERTIES OUTPUT_NAME ${TARGET_NAME})

if (SWA_FLATPAK)
    target_compile_definitions(UnleashedRecomp PRIVATE "GAME_INSTALL_DIRECTORY=\"/var/data\"")
endif()

if (SWA_D3D12)
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directx12-agility CONFIG REQUIRED)
    target_compile_definitions(UnleashedRecomp PRIVATE SWA_D3D12)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_compile_definitions(UnleashedRecomp PRIVATE SDL_VULKAN_ENABLED)
endif()

find_package(directx-dxc REQUIRED)

if (SWA_D3D12)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/D3D12)
    add_custom_command(TARGET UnleashedRecomp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> ${CMAKE_CURRENT_BINARY_DIR}/D3D12
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> ${CMAKE_CURRENT_BINARY_DIR}/D3D12
       COMMAND_EXPAND_LISTS
    )
    
    find_file(DIRECTX_DXIL_LIBRARY "dxil.dll")
    file(COPY ${DIRECTX_DXIL_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    target_link_libraries(UnleashedRecomp PRIVATE
        Microsoft::DirectX-Headers 
        Microsoft::DirectX-Guids 
        Microsoft::DirectX12-Agility
        Microsoft::DirectXShaderCompiler
        Microsoft::DXIL
        dxgi
    )
endif()

file(CHMOD ${DIRECTX_DXC_TOOL} PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

if (WIN32)
    target_link_libraries(UnleashedRecomp PRIVATE
        comctl32
        ntdll
        winmm
        Synchronization
    )
endif()

target_link_libraries(UnleashedRecomp PRIVATE
    fmt::fmt
    libzstd_static
    msdf-atlas-gen::msdf-atlas-gen
    nfd::nfd
    o1heap
    PowerUtils
    SDL2::SDL2-static
    SDL2_mixer
    tomlplusplus::tomlplusplus
    UnleashedRecompLib
    xxHash::xxhash
)

target_include_directories(UnleashedRecomp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/api"
    ${SWA_THIRDPARTY_INCLUDES}
)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(X11 REQUIRED)
    target_include_directories(UnleashedRecomp PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(UnleashedRecomp PRIVATE ${X11_LIBRARIES})
endif()

target_precompile_headers(UnleashedRecomp PUBLIC ${SWA_PRECOMPILED_HEADERS})

function(compile_shader FILE_PATH TARGET_NAME)
    set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gpu/shader/${FILE_PATH}.hlsl)
    cmake_path(GET FILE_PATH STEM VARIABLE_NAME)
    if (SWA_D3D12)
        add_custom_command(
            OUTPUT ${FILE_PATH}.dxil.h
            COMMAND ${DIRECTX_DXC_TOOL} -T ${TARGET_NAME} -HV 2021 -all-resources-bound -Wno-ignored-attributes -Fh ${FILE_PATH}.dxil.h ${FILE_PATH} -Vn g_${VARIABLE_NAME}_dxil
            DEPENDS ${FILE_PATH}
        )
        target_sources(UnleashedRecomp PRIVATE ${FILE_PATH}.dxil.h)
    endif()
    add_custom_command(
        OUTPUT ${FILE_PATH}.spirv.h
        COMMAND ${DIRECTX_DXC_TOOL} -T ${TARGET_NAME} -HV 2021 -all-resources-bound -spirv -fvk-use-dx-layout ${ARGN} -Fh ${FILE_PATH}.spirv.h ${FILE_PATH} -Vn g_${VARIABLE_NAME}_spirv
        DEPENDS ${FILE_PATH}
    )
    target_sources(UnleashedRecomp PRIVATE ${FILE_PATH}.spirv.h)
endfunction()

function(compile_vertex_shader FILE_PATH)
    compile_shader(${FILE_PATH} vs_6_0 -fvk-invert-y)
endfunction()

function(compile_pixel_shader FILE_PATH)
    compile_shader(${FILE_PATH} ps_6_0)
endfunction()

compile_vertex_shader(copy_vs)
compile_pixel_shader(csd_filter_ps)
compile_vertex_shader(csd_no_tex_vs)
compile_vertex_shader(csd_vs)
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

# Only show build type if not Release.
set(IS_BUILD_TYPE_IN_VER_STRING 0)
if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
    set(IS_BUILD_TYPE_IN_VER_STRING 1)
endif()

include("version.cmake")
GenerateVersionSources(
    OUTPUT_DIR "${PROJECT_SOURCE_DIR}"
    VERSION_TXT "${PROJECT_SOURCE_DIR}/res/version.txt"
    H_TEMPLATE "${PROJECT_SOURCE_DIR}/res/version.h.template"
    CXX_TEMPLATE "${PROJECT_SOURCE_DIR}/res/version.cpp.template"
    BUILD_TYPE ${CMAKE_BUILD_TYPE}
    IS_BUILD_TYPE_IN_VER_STRING ${IS_BUILD_TYPE_IN_VER_STRING}
    IS_GIT_REPO 1
)

set(RESOURCES_SOURCE_PATH "${PROJECT_SOURCE_DIR}/../UnleashedRecompResources")
set(RESOURCES_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/res")

BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/bc_diff/button_bc_diff.bin" DEST_FILE "${RESOURCES_OUTPUT_PATH}/bc_diff/button_bc_diff.bin" ARRAY_NAME "g_button_bc_diff" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/font/im_font_atlas.bin" DEST_FILE "${RESOURCES_OUTPUT_PATH}/font/im_font_atlas.bin" ARRAY_NAME "g_im_font_atlas" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/font/im_font_atlas.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/font/im_font_atlas.dds" ARRAY_NAME "g_im_font_atlas_texture" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/achievements_menu/trophy.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/achievements_menu/trophy.dds" ARRAY_NAME "g_trophy" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/controller.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/controller.dds" ARRAY_NAME "g_controller" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/general_window.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/general_window.dds" ARRAY_NAME "g_general_window" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/common/hedge-dev.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/common/hedge-dev.dds" ARRAY_NAME "g_hedgedev" COMPRESSION_TYPE "zstd")
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
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/default.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/default.dds" ARRAY_NAME "g_default" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/effects_volume.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/effects_volume.dds" ARRAY_NAME "g_effects_volume" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/fps.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/fps.dds" ARRAY_NAME "g_fps" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/fullscreen.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/fullscreen.dds" ARRAY_NAME "g_fullscreen" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/gi_texture_filtering_bilinear.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/gi_texture_filtering_bilinear.dds" ARRAY_NAME "g_gi_texture_filtering_bilinear" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/gi_texture_filtering_bicubic.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/gi_texture_filtering_bicubic.dds" ARRAY_NAME "g_gi_texture_filtering_bicubic" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/hints.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/hints.dds" ARRAY_NAME "g_hints" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/invert_camera_x.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/invert_camera_x.dds" ARRAY_NAME "g_invert_camera_x" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/invert_camera_y.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/invert_camera_y.dds" ARRAY_NAME "g_invert_camera_y" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/language.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/language.dds" ARRAY_NAME "g_language" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/master_volume.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/master_volume.dds" ARRAY_NAME "g_master_volume" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/monitor.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/monitor.dds" ARRAY_NAME "g_monitor" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/motion_blur_off.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/motion_blur_off.dds" ARRAY_NAME "g_motion_blur_off" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/motion_blur_original.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/motion_blur_original.dds" ARRAY_NAME "g_motion_blur_original" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/motion_blur_enhanced.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/motion_blur_enhanced.dds" ARRAY_NAME "g_motion_blur_enhanced" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/movie_scale_mode.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/movie_scale_mode.dds" ARRAY_NAME "g_movie_scale_mode" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/music_attenuation.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/music_attenuation.dds" ARRAY_NAME "g_music_attenuation" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/music_volume.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/music_volume.dds" ARRAY_NAME "g_music_volume" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/resolution_scale.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/resolution_scale.dds" ARRAY_NAME "g_resolution_scale" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_original.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_original.dds" ARRAY_NAME "g_shadow_resolution_original" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_x512.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_x512.dds" ARRAY_NAME "g_shadow_resolution_x512" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_x1024.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_x1024.dds" ARRAY_NAME "g_shadow_resolution_x1024" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_x2048.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_x2048.dds" ARRAY_NAME "g_shadow_resolution_x2048" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_x4096.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_x4096.dds" ARRAY_NAME "g_shadow_resolution_x4096" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/shadow_resolution_x8192.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/shadow_resolution_x8192.dds" ARRAY_NAME "g_shadow_resolution_x8192" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/subtitles.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/subtitles.dds" ARRAY_NAME "g_subtitles" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/time_of_day_transition_xbox.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/time_of_day_transition_xbox.dds" ARRAY_NAME "g_time_of_day_transition_xbox" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/time_of_day_transition_playstation.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/time_of_day_transition_playstation.dds" ARRAY_NAME "g_time_of_day_transition_playstation" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/transparency_antialiasing_false.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/transparency_antialiasing_false.dds" ARRAY_NAME "g_transparency_antialiasing_false" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/transparency_antialiasing_true.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/transparency_antialiasing_true.dds" ARRAY_NAME "g_transparency_antialiasing_true" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/ui_scale_mode.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/ui_scale_mode.dds" ARRAY_NAME "g_ui_scale_mode" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/voice_language.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/voice_language.dds" ARRAY_NAME "g_voice_language" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/vibration.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/vibration.dds" ARRAY_NAME "g_vibration" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/vsync.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/vsync.dds" ARRAY_NAME "g_vsync" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/window_size.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/window_size.dds" ARRAY_NAME "g_window_size" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/thumbnails/xbox_color_correction.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/thumbnails/xbox_color_correction.dds" ARRAY_NAME "g_xbox_color_correction" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/options_menu/miles_electric.dds" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/options_menu/miles_electric.dds" ARRAY_NAME "g_miles_electric" COMPRESSION_TYPE "zstd")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/game_icon.bmp" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/game_icon.bmp" ARRAY_NAME "g_game_icon")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/images/game_icon_night.bmp" DEST_FILE "${RESOURCES_OUTPUT_PATH}/images/game_icon_night.bmp" ARRAY_NAME "g_game_icon_night")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_worldmap_cursor.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_worldmap_cursor.ogg" ARRAY_NAME "g_sys_worldmap_cursor")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_worldmap_finaldecide.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_worldmap_finaldecide.ogg" ARRAY_NAME "g_sys_worldmap_finaldecide")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausecansel.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausecansel.ogg" ARRAY_NAME "g_sys_actstg_pausecansel")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausecursor.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausecursor.ogg" ARRAY_NAME "g_sys_actstg_pausecursor")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausedecide.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausedecide.ogg" ARRAY_NAME "g_sys_actstg_pausedecide")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausewinclose.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausewinclose.ogg" ARRAY_NAME "g_sys_actstg_pausewinclose")
BIN2C(TARGET_OBJ UnleashedRecomp SOURCE_FILE "${RESOURCES_SOURCE_PATH}/sounds/sys_actstg_pausewinopen.ogg" DEST_FILE "${RESOURCES_OUTPUT_PATH}/sounds/sys_actstg_pausewinopen.ogg" ARRAY_NAME "g_sys_actstg_pausewinopen")

