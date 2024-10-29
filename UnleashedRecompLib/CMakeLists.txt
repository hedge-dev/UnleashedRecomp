project("UnleashedRecompLib")

add_compile_options(
    "/fp:strict"
    "-march=sandybridge"
    "-fno-strict-aliasing"
)

target_compile_definitions(PowerRecomp PRIVATE CONFIG_FILE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml\")

set(SWA_PPC_RECOMPILED_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_func_mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp_shared.h"
)

foreach(i RANGE 0 260)
    list(APPEND SWA_PPC_RECOMPILED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp.${i}.cpp")
endforeach()

add_custom_command(
    OUTPUT ${SWA_PPC_RECOMPILED_SOURCES}
    COMMAND PowerRecomp
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xex" "${CMAKE_CURRENT_SOURCE_DIR}/config/SWA.toml"
)

set(SHADER_RECOMP_ROOT "${SWA_THIRDPARTY_ROOT}/ShaderRecomp/ShaderRecomp")
set(SHADER_RECOMP_INCLUDE "${SHADER_RECOMP_ROOT}/shader_common.hlsli")

target_compile_definitions(ShaderRecomp PRIVATE 
    SHADER_RECOMP_INPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/private\" 
    SHADER_RECOMP_OUTPUT=\"${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp\"
    SHADER_RECOMP_INCLUDE_INPUT=\"${SHADER_RECOMP_INCLUDE}\"
)

file(GLOB SHADER_RECOMP_SOURCES 
    "${SHADER_RECOMP_ROOT}/*.h"
    "${SHADER_RECOMP_ROOT}/*.cpp"
)

add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
    COMMAND ShaderRecomp
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/private/shader.ar" ${SHADER_RECOMP_SOURCES} ${SHADER_RECOMP_INCLUDE}
)

add_library(UnleashedRecompLib 
    ${SWA_PPC_RECOMPILED_SOURCES}
    "shader/shader_cache.h"
    "shader/shader_cache.cpp"
)

target_include_directories(UnleashedRecompLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(UnleashedRecompLib PUBLIC "ppc/ppc_recomp_shared.h")
