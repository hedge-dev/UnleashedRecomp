# Simple CMake script to compile MoltenVK with SPIRV-Cross as a shared library target

# Prepare MoltenVK Git revision
find_package(Git)

if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        OUTPUT_VARIABLE MVK_GIT_REV
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/MoltenVK
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

set(MVK_GENERATED_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/Generated)
file(WRITE ${MVK_GENERATED_INCLUDES}/mvkGitRevDerived.h "static const char* mvkRevString = \"${MVK_GIT_REV}\";")
message(STATUS "MoltenVK revision: ${MVK_GIT_REV}")

# Prepare MoltenVK version
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/MoltenVK/MoltenVK/MoltenVK/API/mvk_private_api.h MVK_PRIVATE_API)
string(REGEX MATCH "#define MVK_VERSION_MAJOR   [0-9]+" MVK_VERSION_MAJOR_LINE "${MVK_PRIVATE_API}")
string(REGEX MATCH "[0-9]+" MVK_VERSION_MAJOR "${MVK_VERSION_MAJOR_LINE}")
string(REGEX MATCH "#define MVK_VERSION_MINOR   [0-9]+" MVK_VERSION_MINOR_LINE "${MVK_PRIVATE_API}")
string(REGEX MATCH "[0-9]+" MVK_VERSION_MINOR "${MVK_VERSION_MINOR_LINE}")
string(REGEX MATCH "#define MVK_VERSION_PATCH   [0-9]+" MVK_VERSION_PATCH_LINE "${MVK_PRIVATE_API}")
string(REGEX MATCH "[0-9]+" MVK_VERSION_PATCH "${MVK_VERSION_PATCH_LINE}")
set(MVK_VERSION "${MVK_VERSION_MAJOR}.${MVK_VERSION_MINOR}.${MVK_VERSION_PATCH}")
message(STATUS "MoltenVK version: ${MVK_VERSION}")

# Find required system libraries
if(IOS)
    find_library(UIKIT_LIBRARY UIKit REQUIRED)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics REQUIRED)
else()
    find_library(APPKIT_LIBRARY AppKit REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
endif()

find_library(FOUNDATION_LIBRARY Foundation REQUIRED)
find_library(IOSURFACE_LIBRARY IOSurface REQUIRED)
find_library(METAL_LIBRARY Metal REQUIRED)
find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)

# SPIRV-Cross
option(SPIRV_CROSS_CLI "" OFF)
option(SPIRV_CROSS_ENABLE_TESTS "" OFF)
option(SPIRV_CROSS_ENABLE_HLSL "" OFF)
option(SPIRV_CROSS_ENABLE_CPP "" OFF)
option(SPIRV_CROSS_SKIP_INSTALL "" ON)
add_subdirectory(SPIRV-Cross)

# Common
set(MVK_COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MoltenVK/Common)
file(GLOB_RECURSE MVK_COMMON_SOURCES CONFIGURE_DEPENDS
    ${MVK_COMMON_DIR}/*.cpp
    ${MVK_COMMON_DIR}/*.m
    ${MVK_COMMON_DIR}/*.mm)
set(MVK_COMMON_INCLUDES ${MVK_COMMON_DIR})

add_library(MoltenVKCommon STATIC ${MVK_COMMON_SOURCES})
target_include_directories(MoltenVKCommon PUBLIC ${MVK_COMMON_INCLUDES})
target_compile_options(MoltenVKCommon PRIVATE -w)

# MoltenVKShaderConverter
set(MVK_SHADER_CONVERTER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MoltenVK/MoltenVKShaderConverter)
file(GLOB_RECURSE MVK_SHADER_CONVERTER_SOURCES CONFIGURE_DEPENDS
    ${MVK_SHADER_CONVERTER_DIR}/MoltenVKShaderConverter/*.cpp
    ${MVK_SHADER_CONVERTER_DIR}/MoltenVKShaderConverter/*.m
    ${MVK_SHADER_CONVERTER_DIR}/MoltenVKShaderConverter/*.mm)
set(MVK_SHADER_CONVERTER_INCLUDES ${MVK_SHADER_CONVERTER_DIR} ${MVK_SHADER_CONVERTER_DIR}/include)

add_library(MoltenVKShaderConverter STATIC ${MVK_SHADER_CONVERTER_SOURCES})
target_include_directories(MoltenVKShaderConverter PUBLIC ${MVK_SHADER_CONVERTER_INCLUDES})
target_compile_options(MoltenVKShaderConverter PRIVATE -w)
target_link_libraries(MoltenVKShaderConverter PRIVATE spirv-cross-msl spirv-cross-reflect MoltenVKCommon)
target_compile_definitions(MoltenVKShaderConverter PRIVATE MVK_EXCLUDE_SPIRV_TOOLS=1)

# MoltenVK
set(MVK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/MoltenVK/MoltenVK)
file(GLOB_RECURSE MVK_SOURCES CONFIGURE_DEPENDS
    ${MVK_DIR}/MoltenVK/*.cpp
    ${MVK_DIR}/MoltenVK/*.m
    ${MVK_DIR}/MoltenVK/*.mm)
file(GLOB MVK_SRC_INCLUDES LIST_DIRECTORIES ON ${MVK_DIR}/MoltenVK/*)
set(MVK_INCLUDES
    ${MVK_SRC_INCLUDES} ${MVK_GENERATED_INCLUDES} ${MVK_DIR}/include
    ${UNLEASHED_RECOMP_THIRDPARTY_ROOT}/plume/contrib/Vulkan-Headers/include)

add_library(MoltenVK SHARED ${MVK_SOURCES})
target_include_directories(MoltenVK PRIVATE ${MVK_INCLUDES})
target_compile_options(MoltenVK PRIVATE -w)
target_link_libraries(MoltenVK PRIVATE
    ${FOUNDATION_LIBRARY} ${IOSURFACE_LIBRARY} ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY}
    spirv-cross-msl MoltenVKCommon MoltenVKShaderConverter)

if(IOS)
    target_link_libraries(MoltenVK PRIVATE ${UIKIT_LIBRARY} ${COREGRAPHICS_LIBRARY})
else()
    target_link_libraries(MoltenVK PRIVATE ${APPKIT_LIBRARY} ${IOKIT_LIBRARY})
endif()

target_compile_definitions(MoltenVK PRIVATE
    MVK_FRAMEWORK_VERSION=${MVK_VERSION}
    MVK_USE_CEREAL=0
    VK_USE_PLATFORM_METAL_EXT=1)

set_source_files_properties(
    ${MVK_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "-fno-objc-arc"
)

set_target_properties(MoltenVK PROPERTIES
    FRAMEWORK TRUE

    FRAMEWORK_VERSION A

    MACOSX_FRAMEWORK_IDENTIFIER org.moltenvk.MoltenVK
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${MVK_VERSION}
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MVK_VERSION}

    PUBLIC_HEADER "${MVK_PUBLIC_HEADERS}"

    POSITION_INDEPENDENT_CODE ON

    XCODE_ATTRIBUTE_DEFINES_MODULE YES
    XCODE_ATTRIBUTE_DYLIB_INSTALL_NAME_BASE "@rpath"

    BUILD_WITH_INSTALL_RPATH YES
    INSTALL_RPATH "@executable_path/Frameworks;@loader_path/Frameworks"
)

if(IOS)
    set_target_properties(MoltenVK PROPERTIES
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES
        XCODE_ATTRIBUTE_ENABLE_BITCODE NO
        XCODE_ATTRIBUTE_SKIP_INSTALL NO

        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET
        "${CMAKE_OSX_DEPLOYMENT_TARGET}"
    )
endif()
