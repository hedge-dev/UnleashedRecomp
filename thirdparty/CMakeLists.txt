cmake_policy(SET CMP0077 NEW)

set(MSDF_ATLAS_BUILD_STANDALONE OFF)
set(MSDF_ATLAS_USE_SKIA OFF)
set(MSDF_ATLAS_NO_ARTERY_FONT ON)
set(MSDFGEN_DISABLE_PNG ON)

set(SDL2MIXER_DEPS_SHARED OFF)
set(SDL2MIXER_VENDORED ON)
set(SDL2MIXER_FLAC OFF)
set(SDL2MIXER_MOD OFF)
set(SDL2MIXER_MP3 OFF)
set(SDL2MIXER_MIDI OFF)
set(SDL2MIXER_OPUS OFF)
set(SDL2MIXER_VORBIS "VORBISFILE")
set(SDL2MIXER_WAVPACK OFF)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(SDL_VULKAN_ENABLED ON CACHE BOOL "")
endif()

if (WIN32)
    set(D3D12_AGILITY_SDK_ENABLED ON CACHE BOOL "")
endif()

if (WIN32 AND UNLEASHED_RECOMP_ARCHITECTURE STREQUAL "arm64")
    # Workaround for https://github.com/libsdl-org/SDL/issues/16481
    # This also works around an issue caused by SDL2 adjusting the stack
    # probe threshold way too high, causing an immediate crash with LTO
    # on a function moving the stack pointer past the guard page. This
    # issue does not exist with SDL3 as SDL_LIBC is the default.
    set(SDL_LIBC ON CACHE BOOL "" FORCE)
endif()

add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/msdf-atlas-gen")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/nativefiledialog-extended")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/o1heap")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/SDL")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/SDL_mixer")
add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/plume")

if (APPLE)
    add_subdirectory("${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/MoltenVK")
endif()
