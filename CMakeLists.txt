cmake_minimum_required (VERSION 3.20)

if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "VCPKG_ROOT is not defined!")
endif()

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
set(SWA_THIRDPARTY_ROOT ${CMAKE_SOURCE_DIR}/thirdparty)
set(SWA_TOOLS_ROOT ${CMAKE_SOURCE_DIR}/tools)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS OFF)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(${SWA_THIRDPARTY_ROOT})
add_subdirectory(${SWA_TOOLS_ROOT})

project("UnleashedRecomp-ALL")

# Include sub-projects.
add_subdirectory("UnleashedRecompLib")
add_subdirectory("UnleashedRecomp")
