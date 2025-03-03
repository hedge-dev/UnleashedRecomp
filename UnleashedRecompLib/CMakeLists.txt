project("UnleashedRecompLib")

add_compile_options(
    -fno-strict-aliasing
)

if (WIN32)
    add_compile_options(/fp:strict)
else()
    add_compile_options(-ffp-model=strict)
endif()

target_compile_definitions(XenonRecomp PRIVATE 
    XENON_RECOMP_CONFIG_FILE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml\"
    XENON_RECOMP_HEADER_FILE_PATH=\"${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h\")

set(UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_func_mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp_shared.h"
)

foreach(i RANGE 0 260)
    list(APPEND UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp.${i}.cpp")
endforeach()

add_custom_command(
    OUTPUT 
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default_patched.xex"
        ${UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES}
    COMMAND 
        $<TARGET_FILE:XenonRecomp>
    DEPENDS 
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xex"
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xexp"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml"
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar"
    COMMAND
        $<TARGET_FILE:x_decompress> "${CMAKE_CURRENT_SOURCE_DIR}/private/shader.ar" "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shader.ar"
)

set(XENOS_RECOMP_ROOT "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenosRecomp/XenosRecomp")
set(XENOS_RECOMP_INCLUDE "${XENOS_RECOMP_ROOT}/shader_common.h")

target_compile_definitions(XenosRecomp PRIVATE 
    XENOS_RECOMP_INPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/private\" 
    XENOS_RECOMP_OUTPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp\"
    XENOS_RECOMP_INCLUDE_INPUT=\"${XENOS_RECOMP_INCLUDE}\"
    UNLEASHED_RECOMP
)

file(GLOB XENOS_RECOMP_SOURCES 
    "${XENOS_RECOMP_ROOT}/*.h"
    "${XENOS_RECOMP_ROOT}/*.cpp"
)

add_custom_command(
    OUTPUT 
        "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
    COMMAND
        $<TARGET_FILE:XenosRecomp>
    DEPENDS 
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default_patched.xex"
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shader_decompressed.ar" 
        ${XENOS_RECOMP_SOURCES} 
        ${XENOS_RECOMP_INCLUDE}
    USES_TERMINAL
)

add_library(UnleashedRecompLib 
    ${UNLEASHED_RECOMP_PPC_RECOMPILED_SOURCES}
    "shader/shader_cache.h"
    "shader/shader_cache.cpp"
)

target_include_directories(UnleashedRecompLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(UnleashedRecompLib PRIVATE "${UNLEASHED_RECOMP_TOOLS_ROOT}/XenonRecomp/thirdparty/simde")
target_precompile_headers(UnleashedRecompLib PUBLIC "ppc/ppc_recomp_shared.h")
