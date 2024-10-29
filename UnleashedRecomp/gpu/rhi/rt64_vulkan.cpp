//
// RT64
//

#define VMA_IMPLEMENTATION
#define VOLK_IMPLEMENTATION 

#include "rt64_vulkan.h"

#include <algorithm>
#include <cmath>
#include <climits>
#include <unordered_map>

#if DLSS_ENABLED
#   include "render/rt64_dlss.h"
#endif

#ifndef NDEBUG
#   define VULKAN_VALIDATION_LAYER_ENABLED
//#   define VULKAN_OBJECT_NAMES_ENABLED
#endif

// TODO:
// - Fix resource pools.

namespace RT64 {
    // Backend constants.

    // Required buffer alignment for acceleration structures.
    static const uint64_t AccelerationStructureBufferAlignment = 256;

    // Required buffer alignment for shader binding table.
    static const uint64_t ShaderBindingTableAlignment = 256;

    // Controls the maximum amount of native queues the backend will create per queue family.
    // Command queues are created as virtual queues on top of the native queues provided by Vulkan,
    // so they're not under the limit set by the the device or the backend.
    static const uint32_t MaxQueuesPerFamilyCount = 4;

    // Required extensions.

    static const std::unordered_set<std::string> RequiredInstanceExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#   if defined(_WIN64)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#   elif defined(__ANDROID__)
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#   elif defined(__linux__)
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#   endif
    };

    static const std::unordered_set<std::string> OptionalInstanceExtensions = {
        // No optional instance extensions yet.
    };
    
    static const std::unordered_set<std::string> RequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
        VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
#   ifdef VULKAN_OBJECT_NAMES_ENABLED
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#   endif
    };
    
    static const std::unordered_set<std::string> OptionalDeviceExtensions = {
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME,
        VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME,
        VK_KHR_PRESENT_ID_EXTENSION_NAME,
        VK_KHR_PRESENT_WAIT_EXTENSION_NAME,
        VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
    };

    // Common functions.

    static uint32_t roundUp(uint32_t value, uint32_t powerOf2Alignment) {
        return (value + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
    }

    static uint64_t roundUp(uint64_t value, uint64_t powerOf2Alignment) {
        return (value + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
    }

    VkFormat toVk(RenderFormat format) {
        switch (format) {
        case RenderFormat::UNKNOWN:
            return VK_FORMAT_UNDEFINED;
        case RenderFormat::R32G32B32A32_TYPELESS:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RenderFormat::R32G32B32A32_FLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case RenderFormat::R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case RenderFormat::R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case RenderFormat::R32G32B32_TYPELESS:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case RenderFormat::R32G32B32_FLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case RenderFormat::R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case RenderFormat::R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case RenderFormat::R16G16B16A16_TYPELESS:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case RenderFormat::R16G16B16A16_FLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case RenderFormat::R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case RenderFormat::R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case RenderFormat::R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case RenderFormat::R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case RenderFormat::R32G32_TYPELESS:
            return VK_FORMAT_R32G32_SFLOAT;
        case RenderFormat::R32G32_FLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case RenderFormat::R8G8B8A8_TYPELESS:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case RenderFormat::R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case RenderFormat::R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case RenderFormat::R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case RenderFormat::R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case RenderFormat::B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case RenderFormat::R16G16_TYPELESS:
            return VK_FORMAT_R16G16_SFLOAT;
        case RenderFormat::R16G16_FLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case RenderFormat::R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case RenderFormat::R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case RenderFormat::R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case RenderFormat::R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case RenderFormat::R32_TYPELESS:
            return VK_FORMAT_R32_SFLOAT;
        case RenderFormat::D32_FLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case RenderFormat::R32_FLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case RenderFormat::R32_UINT:
            return VK_FORMAT_R32_UINT;
        case RenderFormat::R32_SINT:
            return VK_FORMAT_R32_SINT;
        case RenderFormat::R8G8_TYPELESS:
            return VK_FORMAT_R8G8_UNORM;
        case RenderFormat::R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case RenderFormat::R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case RenderFormat::R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case RenderFormat::R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case RenderFormat::R16_TYPELESS:
            return VK_FORMAT_R16_SFLOAT;
        case RenderFormat::R16_FLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case RenderFormat::D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case RenderFormat::R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case RenderFormat::R16_UINT:
            return VK_FORMAT_R16_UINT;
        case RenderFormat::R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case RenderFormat::R16_SINT:
            return VK_FORMAT_R16_SINT;
        case RenderFormat::R8_TYPELESS:
            return VK_FORMAT_R8_UNORM;
        case RenderFormat::R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case RenderFormat::R8_UINT:
            return VK_FORMAT_R8_UINT;
        case RenderFormat::R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case RenderFormat::R8_SINT:
            return VK_FORMAT_R8_SINT;
        case RenderFormat::BC1_TYPELESS:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case RenderFormat::BC1_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case RenderFormat::BC1_UNORM_SRGB:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case RenderFormat::BC2_TYPELESS:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case RenderFormat::BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case RenderFormat::BC2_UNORM_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
        case RenderFormat::BC3_TYPELESS:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case RenderFormat::BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case RenderFormat::BC3_UNORM_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case RenderFormat::BC4_TYPELESS:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case RenderFormat::BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case RenderFormat::BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
        case RenderFormat::BC5_TYPELESS:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case RenderFormat::BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case RenderFormat::BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
        case RenderFormat::BC6H_TYPELESS:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case RenderFormat::BC6H_UF16:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case RenderFormat::BC6H_SF16:
            return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case RenderFormat::BC7_TYPELESS:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case RenderFormat::BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case RenderFormat::BC7_UNORM_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;
        default:
            assert(false && "Unknown format.");
            return VK_FORMAT_UNDEFINED;
        }
    }

    static VkImageType toImageType(RenderTextureDimension dimension) {
        switch (dimension) {
        case RenderTextureDimension::TEXTURE_1D:
            return VK_IMAGE_TYPE_1D;
        case RenderTextureDimension::TEXTURE_2D:
            return VK_IMAGE_TYPE_2D;
        case RenderTextureDimension::TEXTURE_3D:
            return VK_IMAGE_TYPE_3D;
        default:
            assert(false && "Unknown resource dimension.");
            return VK_IMAGE_TYPE_MAX_ENUM;
        }
    }

    static VkImageViewType toImageViewType(RenderTextureDimension dimension) {
        switch (dimension) {
        case RenderTextureDimension::TEXTURE_1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case RenderTextureDimension::TEXTURE_2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case RenderTextureDimension::TEXTURE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        default:
            assert(false && "Unknown resource dimension.");
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }

    static VkImageViewType toImageViewType(RenderTextureViewDimension dimension) {
        switch (dimension) {
        case RenderTextureViewDimension::TEXTURE_1D:
            return VK_IMAGE_VIEW_TYPE_1D;
        case RenderTextureViewDimension::TEXTURE_2D:
            return VK_IMAGE_VIEW_TYPE_2D;
        case RenderTextureViewDimension::TEXTURE_3D:
            return VK_IMAGE_VIEW_TYPE_3D;
        case RenderTextureViewDimension::TEXTURE_CUBE:
            return VK_IMAGE_VIEW_TYPE_CUBE;
        default:
            assert(false && "Unknown resource dimension.");
            return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
        }
    }

    static VkImageTiling toVk(RenderTextureArrangement arrangement) {
        switch (arrangement) {
        case RenderTextureArrangement::UNKNOWN:
            return VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
        case RenderTextureArrangement::ROW_MAJOR:
            return VkImageTiling::VK_IMAGE_TILING_LINEAR;
        default:
            assert(false && "Unknown texture arrangement.");
            return VkImageTiling::VK_IMAGE_TILING_MAX_ENUM;
        }
    }

    static VkVertexInputRate toVk(RenderInputSlotClassification classification) {
        switch (classification) {
        case RenderInputSlotClassification::PER_VERTEX_DATA:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case RenderInputSlotClassification::PER_INSTANCE_DATA:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
        default:
            assert(false && "Unknown input slot classification.");
            return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    static VkCullModeFlags toVk(RenderCullMode cullMode) {
        switch (cullMode) {
        case RenderCullMode::NONE:
            return VK_CULL_MODE_NONE;
        case RenderCullMode::FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case RenderCullMode::BACK:
            return VK_CULL_MODE_BACK_BIT;
        default:
            assert(false && "Unknown cull mode.");
            return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
        }
    }

    static VkPrimitiveTopology toVk(RenderPrimitiveTopology topology) {
        switch (topology) {
        case RenderPrimitiveTopology::POINT_LIST:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RenderPrimitiveTopology::LINE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RenderPrimitiveTopology::LINE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case RenderPrimitiveTopology::TRIANGLE_LIST:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; 
        case RenderPrimitiveTopology::TRIANGLE_STRIP:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;  
        case RenderPrimitiveTopology::TRIANGLE_FAN:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        default:
            assert(false && "Unknown primitive topology type.");
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }
    }

    static VkBlendFactor toVk(RenderBlend blend) {
        switch (blend) {
        case RenderBlend::ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case RenderBlend::ONE:
            return VK_BLEND_FACTOR_ONE;
        case RenderBlend::SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case RenderBlend::INV_SRC_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case RenderBlend::SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case RenderBlend::INV_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case RenderBlend::DEST_ALPHA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case RenderBlend::INV_DEST_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case RenderBlend::DEST_COLOR:
            return VK_BLEND_FACTOR_DST_COLOR;
        case RenderBlend::INV_DEST_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case RenderBlend::SRC_ALPHA_SAT:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case RenderBlend::BLEND_FACTOR:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case RenderBlend::INV_BLEND_FACTOR:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case RenderBlend::SRC1_COLOR:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case RenderBlend::INV_SRC1_COLOR:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case RenderBlend::SRC1_ALPHA:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case RenderBlend::INV_SRC1_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            assert(false && "Unknown blend factor.");
            return VK_BLEND_FACTOR_MAX_ENUM;
        }
    }

    static VkBlendOp toVk(RenderBlendOperation operation) {
        switch (operation) {
        case RenderBlendOperation::ADD:
            return VK_BLEND_OP_ADD;
        case RenderBlendOperation::SUBTRACT:
            return VK_BLEND_OP_SUBTRACT;
        case RenderBlendOperation::REV_SUBTRACT:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case RenderBlendOperation::MIN:
            return VK_BLEND_OP_MIN;
        case RenderBlendOperation::MAX:
            return VK_BLEND_OP_MAX;
        default:
            assert(false && "Unknown blend operation.");
            return VK_BLEND_OP_MAX_ENUM;
        }
    }

    static VkLogicOp toVk(RenderLogicOperation operation) {
        switch (operation) {
        case RenderLogicOperation::CLEAR:
            return VK_LOGIC_OP_CLEAR;
        case RenderLogicOperation::SET:
            return VK_LOGIC_OP_SET;
        case RenderLogicOperation::COPY:
            return VK_LOGIC_OP_COPY;
        case RenderLogicOperation::COPY_INVERTED:
            return VK_LOGIC_OP_COPY_INVERTED;
        case RenderLogicOperation::NOOP:
            return VK_LOGIC_OP_NO_OP;
        case RenderLogicOperation::INVERT:
            return VK_LOGIC_OP_INVERT;
        case RenderLogicOperation::AND:
            return VK_LOGIC_OP_AND;
        case RenderLogicOperation::NAND:
            return VK_LOGIC_OP_NAND;
        case RenderLogicOperation::OR:
            return VK_LOGIC_OP_OR;
        case RenderLogicOperation::NOR:
            return VK_LOGIC_OP_NOR;
        case RenderLogicOperation::XOR:
            return VK_LOGIC_OP_XOR;
        case RenderLogicOperation::EQUIV:
            return VK_LOGIC_OP_EQUIVALENT;
        case RenderLogicOperation::AND_REVERSE:
            return VK_LOGIC_OP_AND_REVERSE;
        case RenderLogicOperation::AND_INVERTED:
            return VK_LOGIC_OP_AND_INVERTED;
        case RenderLogicOperation::OR_REVERSE:
            return VK_LOGIC_OP_OR_REVERSE;
        case RenderLogicOperation::OR_INVERTED:
            return VK_LOGIC_OP_OR_INVERTED;
        default:
            assert(false && "Unknown logic operation.");
            return VK_LOGIC_OP_MAX_ENUM;
        }
    }

    static VkCompareOp toVk(RenderComparisonFunction function) {
        switch (function) {
        case RenderComparisonFunction::NEVER:
            return VK_COMPARE_OP_NEVER;
        case RenderComparisonFunction::LESS:
            return VK_COMPARE_OP_LESS;
        case RenderComparisonFunction::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case RenderComparisonFunction::LESS_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case RenderComparisonFunction::GREATER:
            return VK_COMPARE_OP_GREATER;
        case RenderComparisonFunction::NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case RenderComparisonFunction::GREATER_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case RenderComparisonFunction::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        default:
            assert(false && "Unknown comparison function.");
            return VK_COMPARE_OP_MAX_ENUM;
        }
    }

    static VkDescriptorType toVk(RenderDescriptorRangeType type) {
        switch (type) {
        case RenderDescriptorRangeType::CONSTANT_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case RenderDescriptorRangeType::FORMATTED_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case RenderDescriptorRangeType::READ_WRITE_FORMATTED_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case RenderDescriptorRangeType::TEXTURE:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case RenderDescriptorRangeType::READ_WRITE_TEXTURE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case RenderDescriptorRangeType::SAMPLER:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case RenderDescriptorRangeType::STRUCTURED_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case RenderDescriptorRangeType::READ_WRITE_STRUCTURED_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case RenderDescriptorRangeType::ACCELERATION_STRUCTURE:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        default:
            assert(false && "Unknown descriptor range type.");
            return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        }
    }

    static VkFilter toVk(RenderFilter filter) {
        switch (filter) {
        case RenderFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case RenderFilter::LINEAR:
            return VK_FILTER_LINEAR;
        default:
            assert(false && "Unknown filter.");
            return VK_FILTER_MAX_ENUM;
        }
    }

    static VkSamplerMipmapMode toVk(RenderMipmapMode mode) {
        switch (mode) {
        case RenderMipmapMode::NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case RenderMipmapMode::LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:
            assert(false && "Unknown mipmap mode.");
            return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
        }
    }

    static VkSamplerAddressMode toVk(RenderTextureAddressMode mode) {
        switch (mode) {
        case RenderTextureAddressMode::WRAP:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case RenderTextureAddressMode::MIRROR:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case RenderTextureAddressMode::CLAMP:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case RenderTextureAddressMode::BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case RenderTextureAddressMode::MIRROR_ONCE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:
            assert(false && "Unknown texture address mode.");
            return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }

    static VkBorderColor toVk(RenderBorderColor color) {
        switch (color) {
        case RenderBorderColor::TRANSPARENT_BLACK:
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case RenderBorderColor::OPAQUE_BLACK:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case RenderBorderColor::OPAQUE_WHITE:
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        default:
            assert(false && "Unknown border color.");
            return VK_BORDER_COLOR_MAX_ENUM;
        }
    }

    static VkAccelerationStructureTypeKHR toVk(RenderAccelerationStructureType type) {
        switch (type) {
        case RenderAccelerationStructureType::TOP_LEVEL:
            return VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        case RenderAccelerationStructureType::BOTTOM_LEVEL:
            return VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        default:
            assert(false && "Unknown acceleration structure type.");
            return VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;
        }
    }
    
    static VkPipelineStageFlags toStageFlags(RenderBarrierStages stages, bool rtSupported) {
        VkPipelineStageFlags flags = 0;

        if (stages & RenderBarrierStage::GRAPHICS) {
            flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
            flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
            flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            flags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        if (stages & RenderBarrierStage::COMPUTE) {
            flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

            if (rtSupported) {
                flags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
                flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
            }
        }

        if (stages & RenderBarrierStage::COPY) {
            flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
            flags |= VK_PIPELINE_STAGE_HOST_BIT;
        }

        return flags;
    }

    static VkShaderStageFlagBits toStage(RenderRaytracingPipelineLibrarySymbolType type) {
        switch (type) {
        case RenderRaytracingPipelineLibrarySymbolType::RAYGEN:
            return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case RenderRaytracingPipelineLibrarySymbolType::MISS:
            return VK_SHADER_STAGE_MISS_BIT_KHR;
        case RenderRaytracingPipelineLibrarySymbolType::CLOSEST_HIT:
            return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case RenderRaytracingPipelineLibrarySymbolType::ANY_HIT:
            return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case RenderRaytracingPipelineLibrarySymbolType::INTERSECTION:
            return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case RenderRaytracingPipelineLibrarySymbolType::CALLABLE:
            return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        default:
            assert(false && "Unknown raytracing pipeline library symbol type.");
            return VkShaderStageFlagBits(0);
        }
    }

    static uint32_t toFamilyIndex(RenderCommandListType type) {
        switch (type) {
        case RenderCommandListType::DIRECT:
            return 0;
        case RenderCommandListType::COMPUTE:
            return 1;
        case RenderCommandListType::COPY:
            return 2;
        default:
            assert(false && "Unknown command list type.");
            return 0;
        }
    }

    static VkIndexType toIndexType(RenderFormat format) {
        switch (format) {
        case RenderFormat::R8_UINT:
            return VK_INDEX_TYPE_UINT8_EXT;
        case RenderFormat::R16_UINT:
            return VK_INDEX_TYPE_UINT16;
        case RenderFormat::R32_UINT:
            return VK_INDEX_TYPE_UINT32;
        default:
            assert(false && "Format is not supported as an index type.");
            return VK_INDEX_TYPE_MAX_ENUM;
        }
    }

    static VkBuildAccelerationStructureFlagsKHR toRTASBuildFlags(bool preferFastBuild, bool preferFastTrace) {
        VkBuildAccelerationStructureFlagsKHR flags = 0;
        flags |= preferFastBuild ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR : 0;
        flags |= preferFastTrace ? VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR : 0;
        return flags;
    }
    
    static VkImageLayout toImageLayout(RenderTextureLayout layout) {
        switch (layout) {
        case RenderTextureLayout::UNKNOWN:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case RenderTextureLayout::GENERAL:
            return VK_IMAGE_LAYOUT_GENERAL;
        case RenderTextureLayout::SHADER_READ:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        case RenderTextureLayout::COLOR_WRITE:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case RenderTextureLayout::DEPTH_WRITE:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case RenderTextureLayout::DEPTH_READ:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        case RenderTextureLayout::COPY_SOURCE:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case RenderTextureLayout::COPY_DEST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case RenderTextureLayout::RESOLVE_SOURCE:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case RenderTextureLayout::RESOLVE_DEST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case RenderTextureLayout::PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:
            assert(false && "Unknown texture layout.");
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }

    static VkComponentSwizzle toVk(RenderSwizzle swizzle) {
        switch (swizzle) {
        case RenderSwizzle::IDENTITY:
            return VK_COMPONENT_SWIZZLE_IDENTITY;
        case RenderSwizzle::ZERO:
            return VK_COMPONENT_SWIZZLE_ZERO;
        case RenderSwizzle::ONE:
            return VK_COMPONENT_SWIZZLE_ONE;
        case RenderSwizzle::R:
            return VK_COMPONENT_SWIZZLE_R;
        case RenderSwizzle::G:
            return VK_COMPONENT_SWIZZLE_G;
        case RenderSwizzle::B:
            return VK_COMPONENT_SWIZZLE_B;
        case RenderSwizzle::A:
            return VK_COMPONENT_SWIZZLE_A;
        default:
            assert(false && "Unknown swizzle type.");
            return VK_COMPONENT_SWIZZLE_IDENTITY;
        }
    }

    static void setObjectName(VkDevice device, VkDebugReportObjectTypeEXT objectType, uint64_t object, const std::string &name) {
#   ifdef VULKAN_OBJECT_NAMES_ENABLED
        VkDebugMarkerObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = objectType;
        nameInfo.object = object;
        nameInfo.pObjectName = name.c_str();
        VkResult res = vkDebugMarkerSetObjectNameEXT(device, &nameInfo);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkDebugMarkerSetObjectNameEXT failed with error code 0x%X.\n", res);
            return;
        }
#   endif
    }

    static void fillSpecInfo(const RenderSpecConstant *specConstants, uint32_t specConstantsCount,
        VkSpecializationInfo &specInfo, VkSpecializationMapEntry *specEntries, uint32_t *specData)
    {
        for (uint32_t i = 0; i < specConstantsCount; i++) {
            VkSpecializationMapEntry &entry = specEntries[i];
            entry.constantID = specConstants[i].index;
            entry.offset = i * sizeof(uint32_t);
            entry.size = sizeof(uint32_t);
            specData[i] = specConstants[i].value;
        }

        specInfo.mapEntryCount = specConstantsCount;
        specInfo.pMapEntries = specEntries;
        specInfo.dataSize = specConstantsCount * sizeof(uint32_t);
        specInfo.pData = specData;
    }

    // Underlying implementation for popcount
    // https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
    static int numberOfSetBits(uint32_t i) {
        i = i - ((i >> 1) & 0x55555555);
        i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
        return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    }

    // VulkanBuffer

    VulkanBuffer::VulkanBuffer(VulkanDevice *device, VulkanPool *pool, const RenderBufferDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->pool = pool;
        this->desc = desc;

        const RenderBufferFlags storageFormattedMask = (RenderBufferFlag::STORAGE | RenderBufferFlag::FORMATTED);
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = desc.size;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::VERTEX) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::INDEX) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::STORAGE) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::CONSTANT) ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::FORMATTED) ? VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT : 0;
        bufferInfo.usage |= ((desc.flags & storageFormattedMask) == storageFormattedMask) ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE) ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR : 0;
        bufferInfo.usage |= (desc.flags & RenderBufferFlag::SHADER_BINDING_TABLE) ? VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR : 0;

        const uint32_t deviceAddressMask = RenderBufferFlag::CONSTANT | RenderBufferFlag::ACCELERATION_STRUCTURE | RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH | RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT | RenderBufferFlag::SHADER_BINDING_TABLE;
        bufferInfo.usage |= (desc.flags & deviceAddressMask) ? VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT : 0;

        VmaAllocationCreateInfo createInfo = {};
        /* TODO: Debug pools.
        createInfo.pool = (pool != nullptr) ? pool->vk : VK_NULL_HANDLE;
        */
        createInfo.usage = VMA_MEMORY_USAGE_AUTO;

        switch (desc.heapType) {
        case RenderHeapType::DEFAULT:
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            createInfo.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case RenderHeapType::UPLOAD:
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            createInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        case RenderHeapType::READBACK:
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            createInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            break;
        default:
            assert(false && "Unknown heap type.");
            break;
        }

        if (desc.committed) {
            createInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        VkDeviceSize minAlignment = 0;

        // The specification imposes an alignment requirement for SBTs.
        if (desc.flags & RenderBufferFlag::SHADER_BINDING_TABLE) {
            minAlignment = device->rtPipelineProperties.shaderGroupBaseAlignment;
        }

        VkResult res;
        if (minAlignment > 0) {
            res = vmaCreateBufferWithAlignment(device->allocator, &bufferInfo, &createInfo, minAlignment, &vk, &allocation, &allocationInfo);
        }
        else {
            res = vmaCreateBuffer(device->allocator, &bufferInfo, &createInfo, &vk, &allocation, &allocationInfo);
        }

        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateBuffer failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanBuffer::~VulkanBuffer() {
        if (vk != VK_NULL_HANDLE) {
            vmaDestroyBuffer(device->allocator, vk, allocation);
        }
    }

    void *VulkanBuffer::map(uint32_t subresource, const RenderRange *readRange) {
        void *data = nullptr;
        VkResult res = vmaMapMemory(device->allocator, allocation, &data);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vmaMapMemory failed with error code 0x%X.\n", res);
            return nullptr;
        }

        return data;
    }

    void VulkanBuffer::unmap(uint32_t subresource, const RenderRange *writtenRange) {
        vmaUnmapMemory(device->allocator, allocation);
    }

    std::unique_ptr<RenderBufferFormattedView> VulkanBuffer::createBufferFormattedView(RenderFormat format) {
        return std::make_unique<VulkanBufferFormattedView>(this, format);
    }

    void VulkanBuffer::setName(const std::string &name) {
        setObjectName(device->vk, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, uint64_t(vk), name);
    }

    uint64_t VulkanBuffer::getDeviceAddress() const {
        VkBufferDeviceAddressInfo info;
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.pNext = nullptr;
        info.buffer = vk;
        return vkGetBufferDeviceAddress(device->vk, &info);
    }

    // VulkanBufferFormattedView

    VulkanBufferFormattedView::VulkanBufferFormattedView(VulkanBuffer *buffer, RenderFormat format) {
        assert(buffer != nullptr);
        assert((buffer->desc.flags & RenderBufferFlag::FORMATTED) && "Buffer must allow formatted views.");

        this->buffer = buffer;

        VkBufferViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
        createInfo.buffer = buffer->vk;
        createInfo.format = toVk(format);
        createInfo.offset = 0;
        createInfo.range = buffer->desc.size;

        VkResult res = vkCreateBufferView(buffer->device->vk, &createInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateBufferView failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanBufferFormattedView::~VulkanBufferFormattedView() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyBufferView(buffer->device->vk, vk, nullptr);
        }
    }

    // VulkanTexture

    VulkanTexture::VulkanTexture(VulkanDevice *device, VulkanPool *pool, const RenderTextureDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->pool = pool;
        this->desc = desc;
        this->ownership = true;

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = toImageType(desc.dimension);
        imageInfo.format = toVk(desc.format);
        imageInfo.extent.width = uint32_t(desc.width);
        imageInfo.extent.height = desc.height;
        imageInfo.extent.depth = desc.depth;
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = desc.arraySize;
        imageInfo.samples = VkSampleCountFlagBits(desc.multisampling.sampleCount);
        imageInfo.tiling = toVk(desc.textureArrangement);
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage |= (desc.flags & RenderTextureFlag::RENDER_TARGET) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
        imageInfo.usage |= (desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : 0;
        imageInfo.usage |= (desc.flags & RenderTextureFlag::STORAGE) ? VK_IMAGE_USAGE_STORAGE_BIT : 0;

        if (desc.multisampling.sampleLocationsEnabled && (desc.flags & RenderTextureFlag::DEPTH_TARGET)) {
            imageInfo.flags |= VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;
        }

        if (desc.flags & RenderTextureFlag::CUBE) {
            imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        }

        imageFormat = imageInfo.format;
        fillSubresourceRange();

        VmaAllocationCreateInfo createInfo = {};
        createInfo.pool = (pool != nullptr) ? pool->vk : VK_NULL_HANDLE;
        createInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        if (desc.committed) {
            createInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        }

        VkResult res = vmaCreateImage(device->allocator, &imageInfo, &createInfo, &vk, &allocation, &allocationInfo);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vmaCreateImage failed with error code 0x%X.\n", res);
            return;
        }

        createImageView(imageInfo.format);
    }

    VulkanTexture::VulkanTexture(VulkanDevice *device, VkImage image) {
        assert(device != nullptr);
        assert(image != VK_NULL_HANDLE);

        this->device = device;
        vk = image;
    }

    VulkanTexture::~VulkanTexture() {
        if (imageView != VK_NULL_HANDLE) {
            vkDestroyImageView(device->vk, imageView, nullptr);
        }

        if (ownership && (vk != VK_NULL_HANDLE)) {
            vmaDestroyImage(device->allocator, vk, allocation);
        }
    }
    
    void VulkanTexture::createImageView(VkFormat format) {
        VkImageView view = VK_NULL_HANDLE;
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = vk;
        viewInfo.viewType = toImageViewType(desc.dimension);
        viewInfo.format = format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange = imageSubresourceRange;
        
        VkResult res = vkCreateImageView(device->vk, &viewInfo, nullptr, &imageView);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView failed with error code 0x%X.\n", res);
            return;
        }
    }

    std::unique_ptr<RenderTextureView> VulkanTexture::createTextureView(const RenderTextureViewDesc &desc) {
        return std::make_unique<VulkanTextureView>(this, desc);
    }

    void VulkanTexture::setName(const std::string &name) {
        setObjectName(device->vk, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, uint64_t(vk), name);
    }
    
    void VulkanTexture::fillSubresourceRange() {
        imageSubresourceRange.aspectMask = (desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageSubresourceRange.baseMipLevel = 0;
        imageSubresourceRange.levelCount = desc.mipLevels;
        imageSubresourceRange.baseArrayLayer = 0;
        imageSubresourceRange.layerCount = 1;
    }

    // VulkanTextureView

    VulkanTextureView::VulkanTextureView(VulkanTexture *texture, const RenderTextureViewDesc &desc) {
        assert(texture != nullptr);

        this->texture = texture;

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = texture->vk;
        viewInfo.viewType = toImageViewType(desc.dimension);
        viewInfo.format = toVk(desc.format);
        viewInfo.components.r = toVk(desc.componentMapping.r);
        viewInfo.components.g = toVk(desc.componentMapping.g);
        viewInfo.components.b = toVk(desc.componentMapping.b);
        viewInfo.components.a = toVk(desc.componentMapping.a);
        viewInfo.subresourceRange.aspectMask = (texture->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = desc.mipSlice;
        viewInfo.subresourceRange.levelCount = desc.mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = texture->desc.arraySize;

        VkResult res = vkCreateImageView(texture->device->vk, &viewInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateImageView failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanTextureView::~VulkanTextureView() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyImageView(texture->device->vk, vk, nullptr);
        }
    }

    // VulkanAccelerationStructure

    VulkanAccelerationStructure::VulkanAccelerationStructure(VulkanDevice *device, const RenderAccelerationStructureDesc &desc) {
        assert(device != nullptr);
        assert(desc.buffer.ref != nullptr);

        this->device = device;
        this->type = desc.type;

        const VulkanBuffer *interfaceBuffer = static_cast<const VulkanBuffer *>(desc.buffer.ref);
        VkAccelerationStructureCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        createInfo.buffer = interfaceBuffer->vk;
        createInfo.offset = desc.buffer.offset;
        createInfo.size = desc.size;
        createInfo.type = toVk(desc.type);

        VkResult res = vkCreateAccelerationStructureKHR(device->vk, &createInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateAccelerationStructureKHR failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanAccelerationStructure::~VulkanAccelerationStructure() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyAccelerationStructureKHR(device->vk, vk, nullptr);
        }
    }

    // VulkanDescriptorSetLayout

    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice *device, const RenderDescriptorSetDesc &descriptorSetDesc) {
        assert(device != nullptr);

        this->device = device;

        // Gather immutable sampler handles.
        thread_local std::vector<VkSampler> samplerHandles;
        samplerHandles.clear();

        for (uint32_t i = 0; i < descriptorSetDesc.descriptorRangesCount; i++) {
            const RenderDescriptorRange &srcRange = descriptorSetDesc.descriptorRanges[i];
            if (srcRange.immutableSampler != nullptr) {
                for (uint32_t j = 0; j < srcRange.count; j++) {
                    const VulkanSampler *interfaceSampler = static_cast<const VulkanSampler *>(srcRange.immutableSampler[j]);
                    assert(interfaceSampler != nullptr);
                    samplerHandles.emplace_back(interfaceSampler->vk);
                }
            }
        }
        
        // Create bindings.
        uint32_t immutableSamplerIndex = 0;
        for (uint32_t i = 0; i < descriptorSetDesc.descriptorRangesCount; i++) {
            const RenderDescriptorRange &srcRange = descriptorSetDesc.descriptorRanges[i];
            VkDescriptorSetLayoutBinding dstBinding = {};
            dstBinding.binding = srcRange.binding;
            dstBinding.descriptorCount = srcRange.count;
            dstBinding.stageFlags = VK_SHADER_STAGE_ALL;
            dstBinding.descriptorType = toVk(srcRange.type);
            if (srcRange.immutableSampler != nullptr) {
                dstBinding.pImmutableSamplers = &samplerHandles[immutableSamplerIndex];
                immutableSamplerIndex += srcRange.count;
            }

            uint32_t indexBase = uint32_t(descriptorIndexBases.size());
            uint32_t bindingIndex = uint32_t(setBindings.size());
            for (uint32_t j = 0; j < srcRange.count; j++) {
                descriptorIndexBases.emplace_back(indexBase);
                descriptorBindingIndices.emplace_back(bindingIndex);
            }

            setBindings.emplace_back(dstBinding);
        }

        VkDescriptorSetLayoutCreateInfo setLayoutInfo = {};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.pBindings = !setBindings.empty() ? setBindings.data() : nullptr;
        setLayoutInfo.bindingCount = uint32_t(setBindings.size());
        
        thread_local std::vector<VkDescriptorBindingFlags> bindingFlags;
        VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo = {};
        if (descriptorSetDesc.lastRangeIsBoundless && (descriptorSetDesc.descriptorRangesCount > 0)) {
            bindingFlags.clear();
            bindingFlags.resize(descriptorSetDesc.descriptorRangesCount, 0);
            bindingFlags[descriptorSetDesc.descriptorRangesCount - 1] = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

            flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            flagsInfo.pBindingFlags = bindingFlags.data();
            flagsInfo.bindingCount = uint32_t(bindingFlags.size());

            setLayoutInfo.pNext = &flagsInfo;
            setLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }
        
        VkResult res = vkCreateDescriptorSetLayout(device->vk, &setLayoutInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateDescriptorSetLayout failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device->vk, vk, nullptr);
        }
    }

    // VulkanPipelineLayout

    VulkanPipelineLayout::VulkanPipelineLayout(VulkanDevice *device, const RenderPipelineLayoutDesc &desc) {
        assert(device != nullptr);

        this->device = device;

        VkPipelineLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        for (uint32_t i = 0; i < desc.pushConstantRangesCount; i++) {
            const RenderPushConstantRange &srcRange = desc.pushConstantRanges[i];
            VkPushConstantRange dstRange = {};
            dstRange.size = srcRange.size;
            dstRange.offset = srcRange.offset;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::VERTEX) ? VK_SHADER_STAGE_VERTEX_BIT : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::GEOMETRY) ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::PIXEL) ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::COMPUTE) ? VK_SHADER_STAGE_COMPUTE_BIT : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::RAYGEN) ? VK_SHADER_STAGE_RAYGEN_BIT_KHR : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::ANY_HIT) ? VK_SHADER_STAGE_ANY_HIT_BIT_KHR : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::CLOSEST_HIT) ? VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::MISS) ? VK_SHADER_STAGE_MISS_BIT_KHR : 0;
            dstRange.stageFlags |= (srcRange.stageFlags & RenderShaderStageFlag::CALLABLE) ? VK_SHADER_STAGE_CALLABLE_BIT_KHR : 0;
            pushConstantRanges.emplace_back(dstRange);
        }

        layoutInfo.pPushConstantRanges = !pushConstantRanges.empty() ? pushConstantRanges.data() : nullptr;
        layoutInfo.pushConstantRangeCount = uint32_t(pushConstantRanges.size());

        thread_local std::vector<VkDescriptorSetLayout> setLayoutHandles;
        setLayoutHandles.clear();

        for (uint32_t i = 0; i < desc.descriptorSetDescsCount; i++) {
            VulkanDescriptorSetLayout *setLayout = new VulkanDescriptorSetLayout(device, desc.descriptorSetDescs[i]);
            descriptorSetLayouts.emplace_back(setLayout);
            setLayoutHandles.emplace_back(setLayout->vk);
        }

        layoutInfo.pSetLayouts = !setLayoutHandles.empty() ? setLayoutHandles.data() : nullptr;
        layoutInfo.setLayoutCount = uint32_t(setLayoutHandles.size());

        VkResult res = vkCreatePipelineLayout(device->vk, &layoutInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreatePipelineLayout failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanPipelineLayout::~VulkanPipelineLayout() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device->vk, vk, nullptr);
        }

        for (VulkanDescriptorSetLayout *setLayout : descriptorSetLayouts) {
            delete setLayout;
        }
    }

    // VulkanShader

    VulkanShader::VulkanShader(VulkanDevice *device, const void *data, uint64_t size, const char *entryPointName, RenderShaderFormat format) {
        assert(device != nullptr);
        assert(data != nullptr);
        assert(size > 0);
        assert(format != RenderShaderFormat::UNKNOWN);
        assert(format == RenderShaderFormat::SPIRV);

        this->device = device;
        this->format = format;
        this->entryPointName = (entryPointName != nullptr) ? std::string(entryPointName) : std::string();

        VkShaderModuleCreateInfo shaderInfo = {};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.pCode = reinterpret_cast<const uint32_t *>(data);
        shaderInfo.codeSize = size;
        VkResult res = vkCreateShaderModule(device->vk, &shaderInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateShaderModule failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanShader::~VulkanShader() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device->vk, vk, nullptr);
        }
    }

    // VulkanSampler

    VulkanSampler::VulkanSampler(VulkanDevice *device, const RenderSamplerDesc &desc) {
        assert(device != nullptr);

        this->device = device;

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.minFilter = toVk(desc.minFilter);
        samplerInfo.magFilter = toVk(desc.magFilter);
        samplerInfo.mipmapMode = toVk(desc.mipmapMode);
        samplerInfo.addressModeU = toVk(desc.addressU);
        samplerInfo.addressModeV = toVk(desc.addressV);
        samplerInfo.addressModeW = toVk(desc.addressW);
        samplerInfo.mipLodBias = desc.mipLODBias;
        samplerInfo.anisotropyEnable = desc.anisotropyEnabled;
        samplerInfo.maxAnisotropy = float(desc.maxAnisotropy);
        samplerInfo.compareEnable = desc.comparisonEnabled;
        samplerInfo.compareOp = toVk(desc.comparisonFunc);
        samplerInfo.minLod = desc.minLOD;
        samplerInfo.maxLod = desc.maxLOD;
        samplerInfo.borderColor = toVk(desc.borderColor);
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        VkResult res = vkCreateSampler(device->vk, &samplerInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSampler failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanSampler::~VulkanSampler() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroySampler(device->vk, vk, nullptr);
        }
    }

    // VulkanPipeline

    VulkanPipeline::VulkanPipeline(VulkanDevice *device, Type type) {
        assert(device != nullptr);
        assert(type != Type::Unknown);

        this->device = device;
        this->type = type;
    }

    VulkanPipeline::~VulkanPipeline() { }

    // VulkanComputePipeline

    VulkanComputePipeline::VulkanComputePipeline(VulkanDevice *device, const RenderComputePipelineDesc &desc) : VulkanPipeline(device, Type::Compute) {
        assert(desc.computeShader != nullptr);
        assert(desc.pipelineLayout != nullptr);

        std::vector<VkSpecializationMapEntry> specEntries(desc.specConstantsCount);
        std::vector<uint32_t> specData(desc.specConstantsCount);
        VkSpecializationInfo specInfo = {};
        fillSpecInfo(desc.specConstants, desc.specConstantsCount, specInfo, specEntries.data(), specData.data());

        const VulkanShader *computeShader = static_cast<const VulkanShader *>(desc.computeShader);
        VkPipelineShaderStageCreateInfo stageInfo = {};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stageInfo.module = computeShader->vk;
        stageInfo.pName = computeShader->entryPointName.c_str();
        stageInfo.pSpecializationInfo = (specInfo.mapEntryCount > 0) ? &specInfo : nullptr;

        const VulkanPipelineLayout *pipelineLayout = static_cast<const VulkanPipelineLayout *>(desc.pipelineLayout);
        VkComputePipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = pipelineLayout->vk;
        pipelineInfo.stage = stageInfo;

        VkResult res = vkCreateComputePipelines(device->vk, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateComputePipelines failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanComputePipeline::~VulkanComputePipeline() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyPipeline(device->vk, vk, nullptr);
        }
    }

    RenderPipelineProgram VulkanComputePipeline::getProgram(const std::string &name) const {
        assert(false && "Compute pipelines can't retrieve shader programs.");
        return RenderPipelineProgram();
    }

    // VulkanGraphicsPipeline

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice *device, const RenderGraphicsPipelineDesc &desc) : VulkanPipeline(device, Type::Graphics) {
        assert(desc.pipelineLayout != nullptr);

        thread_local std::vector<VkPipelineShaderStageCreateInfo> stages;
        stages.clear();

        std::vector<VkSpecializationMapEntry> specEntries(desc.specConstantsCount);
        std::vector<uint32_t> specData(desc.specConstantsCount);
        VkSpecializationInfo specInfo = {};
        fillSpecInfo(desc.specConstants, desc.specConstantsCount, specInfo, specEntries.data(), specData.data());

        const VkSpecializationInfo *pSpecInfo = (specInfo.mapEntryCount > 0) ? &specInfo : nullptr;
        if (desc.vertexShader != nullptr) {
            const VulkanShader *vertexShader = static_cast<const VulkanShader *>(desc.vertexShader);
            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            stageInfo.module = vertexShader->vk;
            stageInfo.pName = vertexShader->entryPointName.c_str();
            stageInfo.pSpecializationInfo = pSpecInfo;
            stages.emplace_back(stageInfo);
        }

        if (desc.geometryShader != nullptr) {
            const VulkanShader *geometryShader = static_cast<const VulkanShader *>(desc.geometryShader);
            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
            stageInfo.module = geometryShader->vk;
            stageInfo.pName = geometryShader->entryPointName.c_str();
            stageInfo.pSpecializationInfo = pSpecInfo;
            stages.emplace_back(stageInfo);
        }

        if (desc.pixelShader != nullptr) {
            const VulkanShader *pixelShader = static_cast<const VulkanShader *>(desc.pixelShader);
            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            stageInfo.module = pixelShader->vk;
            stageInfo.pName = pixelShader->entryPointName.c_str();
            stageInfo.pSpecializationInfo = pSpecInfo;
            stages.emplace_back(stageInfo);
        }

        thread_local std::vector<VkVertexInputBindingDescription> vertexBindings;
        thread_local std::vector<VkVertexInputAttributeDescription> vertexAttributes;
        vertexBindings.clear();
        vertexAttributes.clear();

        for (uint32_t i = 0; i < desc.inputSlotsCount; i++) {
            const RenderInputSlot &inputSlot = desc.inputSlots[i];
            VkVertexInputBindingDescription binding = {};
            binding.binding = inputSlot.index;
            binding.stride = inputSlot.stride;
            binding.inputRate = toVk(inputSlot.classification);
            vertexBindings.emplace_back(binding);
        }

        for (uint32_t i = 0; i < desc.inputElementsCount; i++) {
            const RenderInputElement &inputElement = desc.inputElements[i];
            VkVertexInputAttributeDescription attribute = {};
            attribute.location = inputElement.location;
            attribute.binding = inputElement.slotIndex;
            attribute.format = toVk(inputElement.format);
            attribute.offset = inputElement.alignedByteOffset;
            vertexAttributes.emplace_back(attribute);
        }

        VkPipelineVertexInputStateCreateInfo vertexInput = {};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.pVertexBindingDescriptions = !vertexBindings.empty() ? vertexBindings.data() : nullptr;
        vertexInput.vertexBindingDescriptionCount = uint32_t(vertexBindings.size());
        vertexInput.pVertexAttributeDescriptions = !vertexAttributes.empty() ? vertexAttributes.data() : nullptr;
        vertexInput.vertexAttributeDescriptionCount = uint32_t(vertexAttributes.size());

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = toVk(desc.primitiveTopology);
        if (desc.primitiveTopology == RenderPrimitiveTopology::LINE_STRIP || desc.primitiveTopology == RenderPrimitiveTopology::TRIANGLE_STRIP) {
            inputAssembly.primitiveRestartEnable = VK_TRUE;
        }

        uint32_t renderTargetCount = desc.renderTargetCount;
        if (renderTargetCount == 0 && desc.depthTargetFormat != RenderFormat::UNKNOWN) {
            renderTargetCount = 1;
        }

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = renderTargetCount;
        viewportState.scissorCount = renderTargetCount;

        VkPipelineRasterizationStateCreateInfo rasterization = {};
        rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization.depthClampEnable = !desc.depthClipEnabled;
        rasterization.rasterizerDiscardEnable = VK_FALSE;
        rasterization.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization.lineWidth = 1.0f;
        rasterization.cullMode = toVk(desc.cullMode);
        rasterization.frontFace = VK_FRONT_FACE_CLOCKWISE;

        if (desc.depthBias != 0 || desc.slopeScaledDepthBias != 0.0f) {
            rasterization.depthBiasEnable = true;
            rasterization.depthBiasConstantFactor = float(desc.depthBias);
            rasterization.depthBiasSlopeFactor = desc.slopeScaledDepthBias;
        }

        thread_local std::vector<VkSampleLocationEXT> sampleLocationVector;
        VkSampleLocationsInfoEXT sampleLocationsInfo = {};
        VkPipelineSampleLocationsStateCreateInfoEXT sampleLocations = {};
        const void *multisamplingNext = nullptr;
        if (desc.multisampling.sampleLocationsEnabled) {
            const float *coordinateRange = device->sampleLocationProperties.sampleLocationCoordinateRange;
            const float coordinateBase = coordinateRange[0];
            const float coordinateSpace = (coordinateRange[1] - coordinateRange[0]) / 15.0f;
            sampleLocationVector.resize(desc.multisampling.sampleCount);
            for (uint32_t i = 0; i < desc.multisampling.sampleCount; i++) {
                const RenderMultisamplingLocation &location = desc.multisampling.sampleLocations[i];
                sampleLocationVector[i].x = coordinateBase + (location.x + 8) * coordinateSpace;
                sampleLocationVector[i].y = coordinateBase + (location.y + 8) * coordinateSpace;
            }

            sampleLocationsInfo.sType = VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT;
            sampleLocationsInfo.sampleLocationsPerPixel = VkSampleCountFlagBits(desc.multisampling.sampleCount);
            sampleLocationsInfo.sampleLocationGridSize.width = 1;
            sampleLocationsInfo.sampleLocationGridSize.height = 1;
            sampleLocationsInfo.sampleLocationsCount = uint32_t(sampleLocationVector.size());
            sampleLocationsInfo.pSampleLocations = sampleLocationVector.data();

            sampleLocations.sType = VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT;
            sampleLocations.sampleLocationsEnable = true;
            sampleLocations.sampleLocationsInfo = sampleLocationsInfo;
            multisamplingNext = &sampleLocations;
        }

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.pNext = multisamplingNext;
        multisampling.rasterizationSamples = VkSampleCountFlagBits(desc.multisampling.sampleCount);
        multisampling.alphaToCoverageEnable = desc.alphaToCoverageEnabled;

        thread_local std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        colorBlendAttachments.clear();

        for (uint32_t i = 0; i < desc.renderTargetCount; i++) {
            VkPipelineColorBlendAttachmentState attachment = {};
            const RenderBlendDesc &blendDesc = desc.renderTargetBlend[i];
            attachment.blendEnable = blendDesc.blendEnabled;
            attachment.srcColorBlendFactor = toVk(blendDesc.srcBlend);
            attachment.dstColorBlendFactor = toVk(blendDesc.dstBlend);
            attachment.colorBlendOp = toVk(blendDesc.blendOp);
            attachment.srcAlphaBlendFactor = toVk(blendDesc.srcBlendAlpha);
            attachment.dstAlphaBlendFactor = toVk(blendDesc.dstBlendAlpha);
            attachment.alphaBlendOp = toVk(blendDesc.blendOpAlpha);
            attachment.colorWriteMask = blendDesc.renderTargetWriteMask;
            colorBlendAttachments.emplace_back(attachment);
        }

        VkPipelineColorBlendStateCreateInfo colorBlend = {};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = desc.logicOpEnabled;
        colorBlend.logicOp = toVk(desc.logicOp);
        colorBlend.pAttachments = !colorBlendAttachments.empty() ? colorBlendAttachments.data() : nullptr;
        colorBlend.attachmentCount = uint32_t(colorBlendAttachments.size());
        
        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = desc.depthEnabled;
        depthStencil.depthWriteEnable = desc.depthWriteEnabled;
        depthStencil.depthCompareOp = toVk(desc.depthFunction);
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;

        thread_local std::vector<VkDynamicState> dynamicStates;
        dynamicStates.clear();
        dynamicStates.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStates.emplace_back(VK_DYNAMIC_STATE_SCISSOR);

        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStates.data();
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

        thread_local std::vector<VkFormat> renderTargetFormats;
        renderTargetFormats.resize(desc.renderTargetCount);
        for (uint32_t i = 0; i < desc.renderTargetCount; i++) {
            renderTargetFormats[i] = toVk(desc.renderTargetFormat[i]);
        }

        renderPass = createRenderPass(device, renderTargetFormats.data(), desc.renderTargetCount, toVk(desc.depthTargetFormat), VkSampleCountFlagBits(desc.multisampling.sampleCount));
        if (renderPass == VK_NULL_HANDLE) {
            return;
        }

        const VulkanPipelineLayout *pipelineLayout = static_cast<const VulkanPipelineLayout *>(desc.pipelineLayout);
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pStages = stages.data();
        pipelineInfo.stageCount = uint32_t(stages.size());
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterization;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout->vk;
        pipelineInfo.renderPass = renderPass;

        VkResult res = vkCreateGraphicsPipelines(device->vk, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateGraphicsPipelines failed with error code 0x%X.\n", res);
            return;
        }
    }
    
    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyPipeline(device->vk, vk, nullptr);
        }

        if (renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device->vk, renderPass, nullptr);
        }
    }

    RenderPipelineProgram VulkanGraphicsPipeline::getProgram(const std::string &name) const {
        assert(false && "Graphics pipelines can't retrieve shader programs.");
        return RenderPipelineProgram();
    }

    VkRenderPass VulkanGraphicsPipeline::createRenderPass(VulkanDevice *device, const VkFormat *renderTargetFormat, uint32_t renderTargetCount, VkFormat depthTargetFormat, VkSampleCountFlagBits sampleCount) {
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkSubpassDescription subpass = {};
        VkAttachmentReference depthReference = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        thread_local std::vector<VkAttachmentDescription> attachments;
        thread_local std::vector<VkAttachmentReference> colorReferences;
        attachments.clear();
        colorReferences.clear();
        for (uint32_t i = 0; i < renderTargetCount; i++) {
            VkAttachmentReference reference = {};
            reference.attachment = uint32_t(attachments.size());
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorReferences.emplace_back(reference);

            VkAttachmentDescription attachment = {};
            attachment.format = renderTargetFormat[i];
            attachment.samples = sampleCount;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.emplace_back(attachment);
        }

        subpass.pColorAttachments = !colorReferences.empty() ? colorReferences.data() : nullptr;
        subpass.colorAttachmentCount = uint32_t(colorReferences.size());

        if (depthTargetFormat != VK_FORMAT_UNDEFINED) {
            depthReference.attachment = uint32_t(attachments.size());
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            subpass.pDepthStencilAttachment = &depthReference;

            VkAttachmentDescription attachment = {};
            attachment.format = depthTargetFormat;
            attachment.samples = sampleCount;
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = attachment.loadOp;
            attachment.stencilStoreOp = attachment.storeOp;
            attachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments.emplace_back(attachment);
        }

        VkRenderPassCreateInfo passInfo = {};
        passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        passInfo.pAttachments = !attachments.empty() ? attachments.data() : nullptr;
        passInfo.attachmentCount = uint32_t(attachments.size());
        passInfo.pSubpasses = &subpass;
        passInfo.subpassCount = 1;

        VkResult res = vkCreateRenderPass(device->vk, &passInfo, nullptr, &renderPass);
        if (res == VK_SUCCESS) {
            return renderPass;
        }
        else {
            fprintf(stderr, "vkCreateRenderPass failed with error code 0x%X.\n", res);
            return VK_NULL_HANDLE;
        }
    }

    // VulkanRaytracingPipeline

    VulkanRaytracingPipeline::VulkanRaytracingPipeline(VulkanDevice *device, const RenderRaytracingPipelineDesc &desc, const RenderPipeline *previousPipeline) : VulkanPipeline(device, VulkanPipeline::Type::Raytracing) {
        assert(desc.pipelineLayout != nullptr);
        assert(!desc.stateUpdateEnabled && "State updates are not supported.");

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
        std::unordered_map<std::string, uint32_t> shaderIndices;

        // Prepare all the vectors for the spec constants beforehand so they're not re-allocated.
        std::vector<VkSpecializationMapEntry> specEntries;
        std::vector<uint32_t> specData;
        std::vector<VkSpecializationInfo> specInfo;
        for (uint32_t i = 0; i < desc.librariesCount; i++) {
            const RenderRaytracingPipelineLibrary &library = desc.libraries[i];
            for (uint32_t j = 0; j < library.symbolsCount; j++) {
                const RenderRaytracingPipelineLibrarySymbol &symbol = library.symbols[j];
                if (symbol.specConstantsCount == 0) {
                    continue;
                }

                for (uint32_t i = 0; i < symbol.specConstantsCount; i++) {
                    specEntries.emplace_back();
                    specData.emplace_back();
                }

                specInfo.emplace_back();
            }
        }

        uint32_t specConstantIndex = 0;
        uint32_t specConstantCursor = 0;
        for (uint32_t i = 0; i < desc.librariesCount; i++) {
            const RenderRaytracingPipelineLibrary &library = desc.libraries[i];
            assert(library.shader != nullptr);

            const VulkanShader *interfaceShader = static_cast<const VulkanShader *>(library.shader);
            for (uint32_t j = 0; j < library.symbolsCount; j++) {
                const RenderRaytracingPipelineLibrarySymbol &symbol = library.symbols[j];
                const bool isRaygen = (symbol.type == RenderRaytracingPipelineLibrarySymbolType::RAYGEN);
                const bool isMiss = (symbol.type == RenderRaytracingPipelineLibrarySymbolType::MISS);
                const uint32_t shaderStageIndex = uint32_t(shaderStages.size());
                const char *exportName = (symbol.exportName != nullptr) ? symbol.exportName : symbol.importName;
                if (isRaygen || isMiss) {
                    VkRayTracingShaderGroupCreateInfoKHR groupInfo = {};
                    groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
                    groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                    groupInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
                    groupInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
                    groupInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
                    groupInfo.generalShader = shaderStageIndex;
                    nameProgramMap[std::string(exportName)] = uint32_t(shaderGroups.size());
                    shaderGroups.emplace_back(groupInfo);
                }

                VkPipelineShaderStageCreateInfo stageInfo = {};
                stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                stageInfo.pName = symbol.importName;
                stageInfo.module = interfaceShader->vk;
                stageInfo.stage = toStage(symbol.type);

                if (symbol.specConstantsCount > 0) {
                    stageInfo.pSpecializationInfo = &specInfo[specConstantIndex];
                    fillSpecInfo(symbol.specConstants, symbol.specConstantsCount, specInfo[specConstantIndex], &specEntries[specConstantCursor], &specData[specConstantCursor]);
                    specConstantCursor += symbol.specConstantsCount;
                    specConstantIndex++;
                }

                shaderIndices[std::string(exportName)] = uint32_t(shaderStages.size());
                shaderStages.emplace_back(stageInfo);
            }
        }

        for (uint32_t i = 0; i < desc.hitGroupsCount; i++) {
            auto getShaderIndex = [&](const char *name) {
                if (name != nullptr) {
                    auto it = shaderIndices.find(std::string(name));
                    assert(it != shaderIndices.end());
                    return it->second;
                }
                else {
                    return uint32_t(VK_SHADER_UNUSED_KHR);
                }
            };

            const RenderRaytracingPipelineHitGroup &hitGroup = desc.hitGroups[i];
            VkRayTracingShaderGroupCreateInfoKHR groupInfo = {};
            groupInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
            groupInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            groupInfo.generalShader = VK_SHADER_UNUSED_KHR;
            groupInfo.closestHitShader = getShaderIndex(hitGroup.closestHitName);
            groupInfo.anyHitShader = getShaderIndex(hitGroup.anyHitName);
            groupInfo.intersectionShader = getShaderIndex(hitGroup.intersectionName);
            nameProgramMap[std::string(hitGroup.hitGroupName)] = uint32_t(shaderGroups.size());
            shaderGroups.emplace_back(groupInfo);
        }

        VkRayTracingPipelineInterfaceCreateInfoKHR interfaceInfo = {};
        interfaceInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_INTERFACE_CREATE_INFO_KHR;
        interfaceInfo.maxPipelineRayPayloadSize = desc.maxPayloadSize;
        interfaceInfo.maxPipelineRayHitAttributeSize = desc.maxAttributeSize;

        const VulkanPipelineLayout *pipelineLayout = static_cast<const VulkanPipelineLayout *>(desc.pipelineLayout);
        VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pGroups = shaderGroups.data();
        pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        pipelineInfo.maxPipelineRayRecursionDepth = desc.maxRecursionDepth;
        pipelineInfo.layout = pipelineLayout->vk;

        this->descriptorSetCount = uint32_t(pipelineLayout->descriptorSetLayouts.size());

        VkResult res = vkCreateRayTracingPipelinesKHR(device->vk, nullptr, nullptr, 1, &pipelineInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateRayTracingPipelinesKHR failed with error code 0x%X.\n", res);
            return;
        }

        groupCount = pipelineInfo.groupCount;
    }
    
    VulkanRaytracingPipeline::~VulkanRaytracingPipeline() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyPipeline(device->vk, vk, nullptr);
        }
    }

    RenderPipelineProgram VulkanRaytracingPipeline::getProgram(const std::string &name) const {
        auto it = nameProgramMap.find(name);
        assert((it != nameProgramMap.end()) && "Program must exist in the PSO.");
        return it->second;
    }

    // VulkanDescriptorSet

    VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice *device, const RenderDescriptorSetDesc &desc) {
        assert(device != nullptr);

        this->device = device;

        thread_local std::unordered_map<VkDescriptorType, uint32_t> typeCounts;
        typeCounts.clear();
        
        uint32_t boundlessRangeSize = 0;
        uint32_t rangeCount = desc.descriptorRangesCount;
        if (desc.lastRangeIsBoundless) {
            assert((desc.descriptorRangesCount > 0) && "There must be at least one descriptor set to define the last range as boundless.");

            // Ensure at least one entry is created for boundless ranges.
            boundlessRangeSize = std::max(desc.boundlessRangeSize, 1U);

            const RenderDescriptorRange &lastDescriptorRange = desc.descriptorRanges[desc.descriptorRangesCount - 1];
            typeCounts[toVk(lastDescriptorRange.type)] += boundlessRangeSize;
            rangeCount--;
        }

        for (uint32_t i = 0; i < rangeCount; i++) {
            const RenderDescriptorRange &descriptorRange = desc.descriptorRanges[i];
            typeCounts[toVk(descriptorRange.type)] += descriptorRange.count;
        }

        setLayout = new VulkanDescriptorSetLayout(device, desc);

        descriptorPool = createDescriptorPool(device, typeCounts, desc.lastRangeIsBoundless);
        if (descriptorPool == VK_NULL_HANDLE) {
            return;
        }

        VkDescriptorSetAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = descriptorPool;
        allocateInfo.pSetLayouts = &setLayout->vk;
        allocateInfo.descriptorSetCount = 1;

        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo = {};
        if (desc.lastRangeIsBoundless) {
            countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
            countInfo.pDescriptorCounts = &boundlessRangeSize;
            countInfo.descriptorSetCount = 1;
            allocateInfo.pNext = &countInfo;
        }

        VkResult res = vkAllocateDescriptorSets(device->vk, &allocateInfo, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkAllocateDescriptorSets failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanDescriptorSet::~VulkanDescriptorSet() {
        if (descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device->vk, descriptorPool, nullptr);
        }

        delete setLayout;
    }
    
    void VulkanDescriptorSet::setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, uint64_t bufferSize, const RenderBufferStructuredView *bufferStructuredView, const RenderBufferFormattedView *bufferFormattedView) {
        if (buffer == nullptr) {
            return;
        }

        const VulkanBuffer *interfaceBuffer = static_cast<const VulkanBuffer *>(buffer);
        const VkBufferView *bufferView = nullptr;
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = interfaceBuffer->vk;
        bufferInfo.range = (bufferSize > 0) ? bufferSize : interfaceBuffer->desc.size;

        if (bufferFormattedView != nullptr) {
            assert((bufferStructuredView == nullptr) && "Can't use structured views and formatted views at the same time.");

            const VulkanBufferFormattedView *interfaceBufferFormattedView = static_cast<const VulkanBufferFormattedView *>(bufferFormattedView);
            bufferView = &interfaceBufferFormattedView->vk;
        }
        else if (bufferStructuredView != nullptr) {
            assert((bufferFormattedView == nullptr) && "Can't use structured views and formatted views at the same time.");
            assert(bufferStructuredView->structureByteStride > 0);

            bufferInfo.offset = bufferStructuredView->firstElement * bufferStructuredView->structureByteStride;
        }
        else {
            bufferInfo.offset = 0;
        }

        setDescriptor(descriptorIndex, &bufferInfo, nullptr, bufferView, nullptr);
    }

    void VulkanDescriptorSet::setTexture(uint32_t descriptorIndex, const RenderTexture *texture, const RenderTextureLayout textureLayout, const RenderTextureView *textureView) {
        if (texture == nullptr) {
            return;
        }

        const VulkanTexture *interfaceTexture = static_cast<const VulkanTexture *>(texture);
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = toImageLayout(textureLayout);

        if (textureView != nullptr) {
            const VulkanTextureView *interfaceTextureView = static_cast<const VulkanTextureView *>(textureView);
            imageInfo.imageView = interfaceTextureView->vk;
        }
        else {
            imageInfo.imageView = (interfaceTexture != nullptr) ? interfaceTexture->imageView : VK_NULL_HANDLE;
        }

        setDescriptor(descriptorIndex, nullptr, &imageInfo, nullptr, nullptr);
    }

    void VulkanDescriptorSet::setSampler(uint32_t descriptorIndex, const RenderSampler *sampler) {
        if (sampler == nullptr) {
            return;
        }

        const VulkanSampler *interfaceSampler = static_cast<const VulkanSampler *>(sampler);
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = interfaceSampler->vk;
        setDescriptor(descriptorIndex, nullptr, &imageInfo, nullptr, nullptr);
    }

    void VulkanDescriptorSet::setAccelerationStructure(uint32_t descriptorIndex, const RenderAccelerationStructure *accelerationStructure) {
        if (accelerationStructure == nullptr) {
            return;
        }

        const VulkanAccelerationStructure *interfaceAccelerationStructure = static_cast<const VulkanAccelerationStructure *>(accelerationStructure);
        VkWriteDescriptorSetAccelerationStructureKHR setAccelerationStructure = {};
        setAccelerationStructure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        setAccelerationStructure.pAccelerationStructures = &interfaceAccelerationStructure->vk;
        setAccelerationStructure.accelerationStructureCount = 1;
        setDescriptor(descriptorIndex, nullptr, nullptr, nullptr, &setAccelerationStructure);
    }

    void VulkanDescriptorSet::setDescriptor(uint32_t descriptorIndex, const VkDescriptorBufferInfo *bufferInfo, const VkDescriptorImageInfo *imageInfo, const VkBufferView *texelBufferView, void *pNext) {
        assert(descriptorIndex < setLayout->descriptorBindingIndices.size());

        const uint32_t indexBase = setLayout->descriptorIndexBases[descriptorIndex];
        const uint32_t bindingIndex = setLayout->descriptorBindingIndices[descriptorIndex];
        const VkDescriptorSetLayoutBinding &setLayoutBinding = setLayout->setBindings[bindingIndex];
        VkWriteDescriptorSet writeDescriptor = {};
        writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.pNext = pNext;
        writeDescriptor.dstSet = vk;
        writeDescriptor.dstBinding = setLayoutBinding.binding;
        writeDescriptor.dstArrayElement = descriptorIndex - indexBase;
        writeDescriptor.descriptorCount = 1;
        writeDescriptor.descriptorType = setLayoutBinding.descriptorType;
        writeDescriptor.pBufferInfo = bufferInfo;
        writeDescriptor.pImageInfo = imageInfo;
        writeDescriptor.pTexelBufferView = texelBufferView;

        vkUpdateDescriptorSets(device->vk, 1, &writeDescriptor, 0, nullptr);
    }

    VkDescriptorPool VulkanDescriptorSet::createDescriptorPool(VulkanDevice *device, const std::unordered_map<VkDescriptorType, uint32_t> &typeCounts, bool lastRangeIsBoundless) {
        thread_local std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.clear();

        VkDescriptorPool descriptorPool;
        for (auto it : typeCounts) {
            VkDescriptorPoolSize poolSize = {};
            poolSize.type = it.first;
            poolSize.descriptorCount = it.second;
            poolSizes.emplace_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.pPoolSizes = !poolSizes.empty() ? poolSizes.data() : nullptr;
        poolInfo.poolSizeCount = uint32_t(poolSizes.size());

        if (lastRangeIsBoundless) {
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        }

        VkResult res = vkCreateDescriptorPool(device->vk, &poolInfo, nullptr, &descriptorPool);
        if (res == VK_SUCCESS) {
            return descriptorPool;
        }
        else {
            fprintf(stderr, "vkCreateDescriptorPool failed with error code 0x%X.\n", res);
            return VK_NULL_HANDLE;
        }
    }

    // VulkanSwapChain

    VulkanSwapChain::VulkanSwapChain(VulkanCommandQueue *commandQueue, RenderWindow renderWindow, uint32_t textureCount, RenderFormat format) {
        assert(commandQueue != nullptr);
        assert(textureCount > 0);

        this->commandQueue = commandQueue;
        this->renderWindow = renderWindow;
        this->format = format;

        VkResult res;

#   ifdef _WIN64
        assert(renderWindow != 0);
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hwnd = HWND(renderWindow);
        surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);

        VulkanInterface *renderInterface = commandQueue->device->renderInterface;
        res = vkCreateWin32SurfaceKHR(renderInterface->instance, &surfaceCreateInfo, nullptr, &surface);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateWin32SurfaceKHR failed with error code 0x%X.\n", res);
            return;
        }
#   elif defined(__ANDROID__)
        assert(renderWindow != nullptr);
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = renderWindow;

        VulkanInterface *renderInterface = commandQueue->device->renderInterface;
        res = vkCreateAndroidSurfaceKHR(renderInterface->instance, &surfaceCreateInfo, nullptr, &surface);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateAndroidSurfaceKHR failed with error code 0x%X.\n", res);
            return;
        }
#   elif defined(__linux__)
        assert(renderWindow.display != 0);
        assert(renderWindow.window != 0);
        VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.dpy = renderWindow.display;
        surfaceCreateInfo.window = renderWindow.window;

        VulkanInterface *renderInterface = commandQueue->device->renderInterface;
        res = vkCreateXlibSurfaceKHR(renderInterface->instance, &surfaceCreateInfo, nullptr, &surface);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateXlibSurfaceKHR failed with error code 0x%X.\n", res);
            return;
        }
#   endif

        VkBool32 presentSupported = false;
        VkPhysicalDevice physicalDevice = commandQueue->device->physicalDevice;
        res = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, commandQueue->familyIndex, surface, &presentSupported);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkGetPhysicalDeviceSurfaceSupportKHR failed with error code 0x%X.\n", res);
            return;
        }

        if (!presentSupported) {
            fprintf(stderr, "Command queue does not support present.\n");
            return;
        }

        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

        // Pick an alpha compositing mode
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
            pickedAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        }
        else if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
            pickedAlphaFlag = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        }
        else {
            fprintf(stderr, "No known supported alpha compositing mode\n");
            return;
        }

        // Make sure maxImageCount is never below minImageCount, as it's allowed to be zero.
        surfaceCapabilities.maxImageCount = std::max(surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        // Clamp the requested buffer count between the bounds of the surface capabilities.
        this->textureCount = std::clamp(textureCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());
        immediatePresentModeSupported = std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end();

        // Check if the format we requested is part of the supported surface formats.
        std::vector<VkSurfaceFormatKHR> compatibleSurfaceFormats;
        VkFormat requestedFormat = toVk(format);
        for (uint32_t i = 0; i < surfaceFormatCount; i++) {
            if (surfaceFormats[i].format == requestedFormat) {
                compatibleSurfaceFormats.emplace_back(surfaceFormats[i]);
                break;
            }
        }

        if (compatibleSurfaceFormats.empty()) {
            fprintf(stderr, "No compatible surface formats were found.\n");
            return;
        }

        // Pick the preferred color space, if not available, pick whatever first shows up on the list.
        for (const VkSurfaceFormatKHR &surfaceFormat : compatibleSurfaceFormats) {
            if (surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                pickedSurfaceFormat = surfaceFormat;
                break;
            }
        }

        if (pickedSurfaceFormat.format == VK_FORMAT_UNDEFINED) {
            pickedSurfaceFormat = compatibleSurfaceFormats[0];
        }

        // FIFO is guaranteed to be supported.
        requiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        // Pick an alpha compositing mode, prefer opaque over inherit.
        if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
            pickedAlphaFlag = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        }
        else if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
            pickedAlphaFlag = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
        }
        else {
            fprintf(stderr, "No supported alpha compositing mode was found.\n");
            return;
        }

        // Parent command queue should track this swap chain.
        commandQueue->swapChains.insert(this);
    }

    VulkanSwapChain::~VulkanSwapChain() {
        releaseImageViews();
        releaseSwapChain();

        if (surface != VK_NULL_HANDLE) {
            VulkanInterface *renderInterface = commandQueue->device->renderInterface;
            vkDestroySurfaceKHR(renderInterface->instance, surface, nullptr);
        }

        // Remove tracking from the parent command queue.
        commandQueue->swapChains.erase(this);
    }

    bool VulkanSwapChain::present(uint32_t textureIndex, RenderCommandSemaphore **waitSemaphores, uint32_t waitSemaphoreCount) {
        thread_local std::vector<VkSemaphore> waitSemaphoresVector;
        waitSemaphoresVector.clear();
        for (uint32_t i = 0; i < waitSemaphoreCount; i++) {
            VulkanCommandSemaphore *interfaceSemaphore = (VulkanCommandSemaphore *)(waitSemaphores[i]);
            waitSemaphoresVector.emplace_back(interfaceSemaphore->vk);
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = &vk;
        presentInfo.swapchainCount = 1;
        presentInfo.pImageIndices = &textureIndex;
        presentInfo.pWaitSemaphores = !waitSemaphoresVector.empty() ? waitSemaphoresVector.data() : nullptr;
        presentInfo.waitSemaphoreCount = uint32_t(waitSemaphoresVector.size());
        
        VkResult res;
        {
            const std::scoped_lock queueLock(*commandQueue->queue->mutex);
            res = vkQueuePresentKHR(commandQueue->queue->vk, &presentInfo);
        }

        // Handle the error silently.
        if ((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR)) {
            return false;
        }

        return true;
    }

    bool VulkanSwapChain::resize() {
        getWindowSize(width, height);

        // Don't recreate the swap chain at all if the window doesn't have a valid size.
        if ((width == 0) || (height == 0)) {
            return false;
        }

        // Destroy any image view references to the current swap chain.
        releaseImageViews();

        // We don't actually need to query the surface capabilities but the validation layer seems to cache the valid extents from this call.
        VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(commandQueue->device->physicalDevice, surface, &surfaceCapabilities);

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = textureCount;
        createInfo.imageFormat = pickedSurfaceFormat.format;
        createInfo.imageColorSpace = pickedSurfaceFormat.colorSpace;
        createInfo.imageExtent.width = width;
        createInfo.imageExtent.height = height;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.compositeAlpha = pickedAlphaFlag;
        createInfo.presentMode = requiredPresentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = vk;

        VkResult res = vkCreateSwapchainKHR(commandQueue->device->vk, &createInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSwapchainKHR failed with error code 0x%X.\n", res);
            return false;
        }

        // Store the chosen present mode to identify later whether the swapchain needs to be recreated.
        createdPresentMode = requiredPresentMode;

        // Reset present counter.
        presentCount = 1;

        if (createInfo.oldSwapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(commandQueue->device->vk, createInfo.oldSwapchain, nullptr);
        }

        uint32_t retrievedImageCount = 0;
        vkGetSwapchainImagesKHR(commandQueue->device->vk, vk, &retrievedImageCount, nullptr);
        if (retrievedImageCount < textureCount) {
            releaseSwapChain();
            fprintf(stderr, "Image count differs from the texture count.\n");
            return false;
        }

        textureCount = retrievedImageCount;

        std::vector<VkImage> images(textureCount);
        res = vkGetSwapchainImagesKHR(commandQueue->device->vk, vk, &textureCount, images.data());
        if (res != VK_SUCCESS) {
            releaseSwapChain();
            fprintf(stderr, "vkGetSwapchainImagesKHR failed with error code 0x%X.\n", res);
            return false;
        }

        // Assign the swap chain images to the buffer resources.
        textures.resize(textureCount);

        for (uint32_t i = 0; i < textureCount; i++) {
            textures[i] = VulkanTexture(commandQueue->device, images[i]);
            textures[i].desc.dimension = RenderTextureDimension::TEXTURE_2D;
            textures[i].desc.format = format;
            textures[i].desc.width = width;
            textures[i].desc.height = height;
            textures[i].desc.depth = 1;
            textures[i].desc.mipLevels = 1;
            textures[i].desc.arraySize = 1;
            textures[i].desc.flags = RenderTextureFlag::RENDER_TARGET;
            textures[i].fillSubresourceRange();
            textures[i].createImageView(pickedSurfaceFormat.format);
        }

        return true;
    }

    bool VulkanSwapChain::needsResize() const {
        uint32_t windowWidth, windowHeight;
        getWindowSize(windowWidth, windowHeight);
        return (vk == VK_NULL_HANDLE) || (windowWidth != width) || (windowHeight != height) || (requiredPresentMode != createdPresentMode);
    }

    void VulkanSwapChain::setVsyncEnabled(bool vsyncEnabled) {
        // Immediate mode must be supported and the presentation mode will only be used on the next resize.
        // needsResize() will return as true as long as the created and required present mode do not match.
        if (immediatePresentModeSupported) {
            requiredPresentMode = vsyncEnabled ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }

    bool VulkanSwapChain::isVsyncEnabled() const {
        return createdPresentMode == VK_PRESENT_MODE_FIFO_KHR;
    }

    uint32_t VulkanSwapChain::getWidth() const {
        return width;
    }

    uint32_t VulkanSwapChain::getHeight() const {
        return height;
    }

    RenderTexture *VulkanSwapChain::getTexture(uint32_t textureIndex) {
        return &textures[textureIndex];
    }

    uint32_t VulkanSwapChain::getTextureCount() const {
        return textureCount;
    }

    RenderWindow VulkanSwapChain::getWindow() const {
        return renderWindow;
    }

    bool VulkanSwapChain::isEmpty() const {
        return (vk == VK_NULL_HANDLE) || (width == 0) || (height == 0);
    }

    uint32_t VulkanSwapChain::getRefreshRate() const {
        VkRefreshCycleDurationGOOGLE refreshCycle = {};
        VkResult res = vkGetRefreshCycleDurationGOOGLE(commandQueue->device->vk, vk, &refreshCycle);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkGetRefreshCycleDurationGOOGLE failed with error code 0x%X.\n", res);
            return 0;
        }

        return std::lround(1000000000.0 / refreshCycle.refreshDuration);
    }

    void VulkanSwapChain::getWindowSize(uint32_t &dstWidth, uint32_t &dstHeight) const {
#   if defined(_WIN64)
        RECT rect;
        GetClientRect(renderWindow, &rect);
        dstWidth = rect.right - rect.left;
        dstHeight = rect.bottom - rect.top;
#   elif defined(__ANDROID__)
        dstWidth = ANativeWindow_getWidth(renderWindow);
        dstHeight = ANativeWindow_getHeight(renderWindow);
#   elif defined(__linux__)
        XWindowAttributes attributes;
        XGetWindowAttributes(renderWindow.display, renderWindow.window, &attributes);
        // The attributes width and height members do not include the border.
        dstWidth = attributes.width;
        dstHeight = attributes.height;
#   endif
    }

    bool VulkanSwapChain::acquireTexture(RenderCommandSemaphore *signalSemaphore, uint32_t *textureIndex) {
        assert(signalSemaphore != nullptr);

        VulkanCommandSemaphore *interfaceSemaphore = static_cast<VulkanCommandSemaphore *>(signalSemaphore);
        VkResult res = vkAcquireNextImageKHR(commandQueue->device->vk, vk, UINT64_MAX, interfaceSemaphore->vk, VK_NULL_HANDLE, textureIndex);
        if ((res != VK_SUCCESS) && (res != VK_SUBOPTIMAL_KHR)) {
            return false;
        }

        return true;
    }

    void VulkanSwapChain::releaseSwapChain() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(commandQueue->device->vk, vk, nullptr);
            vk = VK_NULL_HANDLE;
        }
    }

    void VulkanSwapChain::releaseImageViews() {
        for (VulkanTexture &texture : textures) {
            if (texture.imageView != VK_NULL_HANDLE) {
                vkDestroyImageView(commandQueue->device->vk, texture.imageView, nullptr);
                texture.imageView = VK_NULL_HANDLE;
            }
        }
    }

    // VulkanFramebuffer
    
    VulkanFramebuffer::VulkanFramebuffer(VulkanDevice *device, const RenderFramebufferDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        depthAttachmentReadOnly = desc.depthAttachmentReadOnly;

        VkResult res;
        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colorReferences;
        std::vector<VkImageView> imageViews;
        VkAttachmentReference depthReference = {};
        for (uint32_t i = 0; i < desc.colorAttachmentsCount; i++) {
            const VulkanTexture *colorAttachment = static_cast<const VulkanTexture *>(desc.colorAttachments[i]);
            assert((colorAttachment->desc.flags & RenderTextureFlag::RENDER_TARGET) && "Color attachment must be a render target.");
            colorAttachments.emplace_back(colorAttachment);
            imageViews.emplace_back(colorAttachment->imageView);

            if (i == 0) {
                width = uint32_t(colorAttachment->desc.width);
                height = colorAttachment->desc.height;
            }

            VkAttachmentReference reference = {};
            reference.attachment = uint32_t(attachments.size());
            reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorReferences.emplace_back(reference);

            VkAttachmentDescription attachment = {};
            attachment.format = toVk(colorAttachment->desc.format);
            attachment.samples = VkSampleCountFlagBits(colorAttachment->desc.multisampling.sampleCount);
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attachments.emplace_back(attachment);
        }

        if (desc.depthAttachment != nullptr) {
            depthAttachment = static_cast<const VulkanTexture *>(desc.depthAttachment);
            assert((depthAttachment->desc.flags & RenderTextureFlag::DEPTH_TARGET) && "Depth attachment must be a depth target.");
            imageViews.emplace_back(depthAttachment->imageView);

            if (desc.colorAttachmentsCount == 0) {
                width = uint32_t(depthAttachment->desc.width);
                height = depthAttachment->desc.height;
            }

            depthReference.attachment = uint32_t(attachments.size());
            depthReference.layout = desc.depthAttachmentReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            // Upgrade the operations to NONE if supported. Fixes the following validation issue: https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/2349
            // We prefer to just ignore this potential hazard on older Vulkan versions as it just seems to be an edge case for some hardware.
            const bool preferNoneForReadOnly = desc.depthAttachmentReadOnly && device->loadStoreOpNoneSupported;
            VkAttachmentDescription attachment = {};
            attachment.format = toVk(depthAttachment->desc.format);
            attachment.samples = VkSampleCountFlagBits(depthAttachment->desc.multisampling.sampleCount);
            attachment.loadOp = preferNoneForReadOnly ? VK_ATTACHMENT_LOAD_OP_NONE_EXT : VK_ATTACHMENT_LOAD_OP_LOAD;
            attachment.storeOp = preferNoneForReadOnly ? VK_ATTACHMENT_STORE_OP_NONE_EXT : VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = attachment.loadOp;
            attachment.stencilStoreOp = attachment.storeOp;
            attachment.initialLayout = depthReference.layout;
            attachment.finalLayout = depthReference.layout;
            attachments.emplace_back(attachment);
        }

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = !colorReferences.empty() ? colorReferences.data() : nullptr;
        subpass.colorAttachmentCount = uint32_t(colorReferences.size());

        if (desc.depthAttachment != nullptr) {
            subpass.pDepthStencilAttachment = &depthReference;
        }

        VkRenderPassCreateInfo passInfo = {};
        passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        passInfo.pAttachments = attachments.data();
        passInfo.attachmentCount = uint32_t(attachments.size());
        passInfo.pSubpasses = &subpass;
        passInfo.subpassCount = 1;

        res = vkCreateRenderPass(device->vk, &passInfo, nullptr, &renderPass);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateRenderPass failed with error code 0x%X.\n", res);
            return;
        }
        
        VkFramebufferCreateInfo fbInfo = {};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.renderPass = renderPass;
        fbInfo.pAttachments = imageViews.data();
        fbInfo.attachmentCount = uint32_t(imageViews.size());
        fbInfo.width = width;
        fbInfo.height = height;
        fbInfo.layers = 1;

        res = vkCreateFramebuffer(device->vk, &fbInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateFramebuffer failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanFramebuffer::~VulkanFramebuffer() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device->vk, vk, nullptr);
        }

        if (renderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device->vk, renderPass, nullptr);
        }
    }

    uint32_t VulkanFramebuffer::getWidth() const {
        return width;
    }

    uint32_t VulkanFramebuffer::getHeight() const {
        return height;
    }

    bool VulkanFramebuffer::contains(const VulkanTexture *attachment) const {
        assert(attachment != nullptr);

        for (uint32_t i = 0; i < colorAttachments.size(); i++) {
            if (colorAttachments[i] == attachment) {
                return true;
            }
        }

        return (depthAttachment == attachment);
    }

    // VulkanCommandList

    VulkanCommandList::VulkanCommandList(VulkanDevice *device, RenderCommandListType type) {
        assert(device != nullptr);
        assert(type != RenderCommandListType::UNKNOWN);

        this->device = device;
        this->type = type;

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = device->queueFamilyIndices[toFamilyIndex(type)];

        VkResult res = vkCreateCommandPool(device->vk, &poolInfo, nullptr, &commandPool);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateCommandPool failed with error code 0x%X.\n", res);
            return;
        }

        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = commandPool;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;

        res = vkAllocateCommandBuffers(device->vk, &allocateInfo, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkAllocateCommandBuffers failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanCommandList::~VulkanCommandList() {
        if (vk != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(device->vk, commandPool, 1, &vk);
        }

        if (commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device->vk, commandPool, nullptr);
        }
    }

    void VulkanCommandList::begin() {
        vkResetCommandBuffer(vk, 0);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkResult res = vkBeginCommandBuffer(vk, &beginInfo);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkBeginCommandBuffer failed with error code 0x%X.\n", res);
            return;
        }
    }

    void VulkanCommandList::end() {
        endActiveRenderPass();

        VkResult res = vkEndCommandBuffer(vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkEndCommandBuffer failed with error code 0x%X.\n", res);
            return;
        }

        targetFramebuffer = nullptr;
        activeComputePipelineLayout = nullptr;
        activeGraphicsPipelineLayout = nullptr;
        activeRaytracingPipelineLayout = nullptr;
    }

    void VulkanCommandList::barriers(RenderBarrierStages stages, const RenderBufferBarrier *bufferBarriers, uint32_t bufferBarriersCount, const RenderTextureBarrier *textureBarriers, uint32_t textureBarriersCount) {
        assert((bufferBarriersCount == 0) || (bufferBarriers != nullptr));
        assert((textureBarriersCount == 0) || (textureBarriers != nullptr));

        if ((bufferBarriersCount == 0) && (textureBarriersCount == 0)) {
            return;
        }

        endActiveRenderPass();

        const bool rtEnabled = device->capabilities.raytracing;
        VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | toStageFlags(stages, rtEnabled);
        thread_local std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;
        thread_local std::vector<VkImageMemoryBarrier> imageMemoryBarriers;
        bufferMemoryBarriers.clear();
        imageMemoryBarriers.clear();

        for (uint32_t i = 0; i < bufferBarriersCount; i++) {
            const RenderBufferBarrier &bufferBarrier = bufferBarriers[i];
            VulkanBuffer *interfaceBuffer = static_cast<VulkanBuffer *>(bufferBarrier.buffer);
            VkBufferMemoryBarrier bufferMemoryBarrier = {};
            bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufferMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // TODO
            bufferMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // TODO
            bufferMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufferMemoryBarrier.buffer = interfaceBuffer->vk;
            bufferMemoryBarrier.offset = 0;
            bufferMemoryBarrier.size = interfaceBuffer->desc.size;
            bufferMemoryBarriers.emplace_back(bufferMemoryBarrier);
            srcStageMask |= toStageFlags(interfaceBuffer->barrierStages, rtEnabled);
            interfaceBuffer->barrierStages = stages;
        }

        for (uint32_t i = 0; i < textureBarriersCount; i++) {
            const RenderTextureBarrier &textureBarrier = textureBarriers[i];
            VulkanTexture *interfaceTexture = static_cast<VulkanTexture *>(textureBarrier.texture);
            VkImageMemoryBarrier imageMemoryBarrier = {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = interfaceTexture->vk;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // TODO
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // TODO
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = toImageLayout(interfaceTexture->textureLayout);
            imageMemoryBarrier.newLayout = toImageLayout(textureBarrier.layout);
            imageMemoryBarrier.subresourceRange.levelCount = interfaceTexture->desc.mipLevels;
            imageMemoryBarrier.subresourceRange.layerCount = interfaceTexture->desc.arraySize;
            imageMemoryBarrier.subresourceRange.aspectMask = (interfaceTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarriers.emplace_back(imageMemoryBarrier);
            srcStageMask |= toStageFlags(interfaceTexture->barrierStages, rtEnabled);
            interfaceTexture->textureLayout = textureBarrier.layout;
            interfaceTexture->barrierStages = stages;
        }
        
        if (bufferMemoryBarriers.empty() && imageMemoryBarriers.empty()) {
            return;
        }

        vkCmdPipelineBarrier(vk, srcStageMask, dstStageMask, 0, 0, nullptr, uint32_t(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), uint32_t(imageMemoryBarriers.size()), imageMemoryBarriers.data());
    }

    void VulkanCommandList::dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) {
        vkCmdDispatch(vk, threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    void VulkanCommandList::traceRays(uint32_t width, uint32_t height, uint32_t depth, RenderBufferReference shaderBindingTable, const RenderShaderBindingGroupsInfo &shaderBindingGroupsInfo) {
        const VulkanBuffer *interfaceBuffer = static_cast<const VulkanBuffer *>(shaderBindingTable.ref);
        assert(interfaceBuffer != nullptr);
        assert((interfaceBuffer->desc.flags & RenderBufferFlag::SHADER_BINDING_TABLE) && "Buffer must allow being used as a shader binding table.");

        VkBufferDeviceAddressInfo tableAddressInfo = {};
        tableAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        tableAddressInfo.buffer = interfaceBuffer->vk;

        const VkDeviceAddress tableAddress = vkGetBufferDeviceAddress(device->vk, &tableAddressInfo) + shaderBindingTable.offset;
        const RenderShaderBindingGroupInfo &rayGen = shaderBindingGroupsInfo.rayGen;
        const RenderShaderBindingGroupInfo &miss = shaderBindingGroupsInfo.miss;
        const RenderShaderBindingGroupInfo &hitGroup = shaderBindingGroupsInfo.hitGroup;
        const RenderShaderBindingGroupInfo &callable = shaderBindingGroupsInfo.callable;
        VkStridedDeviceAddressRegionKHR rayGenSbt = {};
        VkStridedDeviceAddressRegionKHR missSbt = {};
        VkStridedDeviceAddressRegionKHR hitSbt = {};
        VkStridedDeviceAddressRegionKHR callableSbt = {};
        rayGenSbt.deviceAddress = (rayGen.size > 0) ? (tableAddress + rayGen.offset + rayGen.startIndex * rayGen.stride) : 0;
        rayGenSbt.size = rayGen.stride; // RayGen is a special case where the size must be the same as the stride.
        rayGenSbt.stride = rayGen.stride;
        missSbt.deviceAddress = (miss.size > 0) ? (tableAddress + miss.offset + miss.startIndex * miss.stride) : 0;
        missSbt.size = miss.size;
        missSbt.stride = miss.stride;
        hitSbt.deviceAddress = (hitGroup.size > 0) ? (tableAddress + hitGroup.offset + hitGroup.startIndex * hitGroup.stride) : 0;
        hitSbt.size = hitGroup.size;
        hitSbt.stride = hitGroup.stride;
        callableSbt.deviceAddress = (callable.size > 0) ? (tableAddress + callable.offset + callable.startIndex * callable.stride) : 0;
        callableSbt.size = callable.size;
        callableSbt.stride = callable.stride;
        vkCmdTraceRaysKHR(vk, &rayGenSbt, &missSbt, &hitSbt, &callableSbt, width, height, depth);
    }

    void VulkanCommandList::drawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) {
        checkActiveRenderPass();

        vkCmdDraw(vk, vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }
    
    void VulkanCommandList::drawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) {
        checkActiveRenderPass();

        vkCmdDrawIndexed(vk, indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void VulkanCommandList::setPipeline(const RenderPipeline *pipeline) {
        assert(pipeline != nullptr);

        const VulkanPipeline *interfacePipeline = static_cast<const VulkanPipeline *>(pipeline);
        switch (interfacePipeline->type) {
        case VulkanPipeline::Type::Compute: {
            const VulkanComputePipeline *computePipeline = static_cast<const VulkanComputePipeline *>(interfacePipeline);
            vkCmdBindPipeline(vk, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->vk);
            break;
        }
        case VulkanPipeline::Type::Graphics: {
            const VulkanGraphicsPipeline *graphicsPipeline = static_cast<const VulkanGraphicsPipeline *>(interfacePipeline);
            vkCmdBindPipeline(vk, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->vk);
            break;
        }
        case VulkanPipeline::Type::Raytracing: {
            const VulkanRaytracingPipeline *raytracingPipeline = static_cast<const VulkanRaytracingPipeline *>(interfacePipeline);
            vkCmdBindPipeline(vk, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline->vk);
            break;
        }
        default:
            assert(false && "Unknown pipeline type.");
            break;
        }
    }

    void VulkanCommandList::setComputePipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        assert(pipelineLayout != nullptr);

        activeComputePipelineLayout = static_cast<const VulkanPipelineLayout *>(pipelineLayout);
    }

    void VulkanCommandList::setComputePushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        assert(activeComputePipelineLayout != nullptr);
        assert(rangeIndex < activeComputePipelineLayout->pushConstantRanges.size());
        
        const VkPushConstantRange &range = activeComputePipelineLayout->pushConstantRanges[rangeIndex];
        vkCmdPushConstants(vk, activeComputePipelineLayout->vk, range.stageFlags & VK_SHADER_STAGE_COMPUTE_BIT, range.offset + offset, size == 0 ? range.size : size, data);
    }

    void VulkanCommandList::setComputeDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setDescriptorSet(VK_PIPELINE_BIND_POINT_COMPUTE, activeComputePipelineLayout, descriptorSet, setIndex);
    }

    void VulkanCommandList::setGraphicsPipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        assert(pipelineLayout != nullptr);

        activeGraphicsPipelineLayout = static_cast<const VulkanPipelineLayout *>(pipelineLayout);
    }

    void VulkanCommandList::setGraphicsPushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        assert(activeGraphicsPipelineLayout != nullptr);
        assert(rangeIndex < activeGraphicsPipelineLayout->pushConstantRanges.size());

        const VkPushConstantRange &range = activeGraphicsPipelineLayout->pushConstantRanges[rangeIndex];
        vkCmdPushConstants(vk, activeGraphicsPipelineLayout->vk, range.stageFlags & VK_SHADER_STAGE_ALL_GRAPHICS, range.offset + offset, size == 0 ? range.size : size, data);
    }

    void VulkanCommandList::setGraphicsDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setDescriptorSet(VK_PIPELINE_BIND_POINT_GRAPHICS, activeGraphicsPipelineLayout, descriptorSet, setIndex);
    }

    void VulkanCommandList::setGraphicsRootDescriptor(RenderBufferReference bufferReference, uint32_t rootDescriptorIndex) {
        assert(false && "Root descriptors are not supported in Vulkan.");
    }

    void VulkanCommandList::setRaytracingPipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        assert(pipelineLayout != nullptr);

        activeRaytracingPipelineLayout = static_cast<const VulkanPipelineLayout *>(pipelineLayout);
    }

    void VulkanCommandList::setRaytracingPushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        assert(activeRaytracingPipelineLayout != nullptr);
        assert(rangeIndex < activeRaytracingPipelineLayout->pushConstantRanges.size());

        const VkPushConstantRange &range = activeRaytracingPipelineLayout->pushConstantRanges[rangeIndex];
        const VkShaderStageFlags raytracingStageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
            VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR;

        vkCmdPushConstants(vk, activeRaytracingPipelineLayout->vk, range.stageFlags & raytracingStageFlags, range.offset + offset, size == 0 ? range.size : size, data);
    }

    void VulkanCommandList::setRaytracingDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setDescriptorSet(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, activeRaytracingPipelineLayout, descriptorSet, setIndex);
    }

    void VulkanCommandList::setIndexBuffer(const RenderIndexBufferView *view) {
        if (view != nullptr) {
            const VulkanBuffer *interfaceBuffer = static_cast<const VulkanBuffer *>(view->buffer.ref);
            vkCmdBindIndexBuffer(vk, (interfaceBuffer != nullptr) ? interfaceBuffer->vk : VK_NULL_HANDLE, view->buffer.offset, toIndexType(view->format));
        }
    }

    void VulkanCommandList::setVertexBuffers(uint32_t startSlot, const RenderVertexBufferView *views, uint32_t viewCount, const RenderInputSlot *inputSlots) {
        if ((views != nullptr) && (viewCount > 0)) {
            // Input slots aren't actually used by Vulkan as the stride is baked into the pipeline, but we validate it for the sake of consistency with D3D12.
            assert(inputSlots != nullptr);

            thread_local std::vector<VkBuffer> bufferVector;
            thread_local std::vector<VkDeviceSize> offsetVector;
            bufferVector.clear();
            offsetVector.clear();
            for (uint32_t i = 0; i < viewCount; i++) {
                const VulkanBuffer *interfaceBuffer = static_cast<const VulkanBuffer *>(views[i].buffer.ref);
                bufferVector.emplace_back((interfaceBuffer != nullptr) ? interfaceBuffer->vk : VK_NULL_HANDLE);
                offsetVector.emplace_back(views[i].buffer.offset);
            }

            vkCmdBindVertexBuffers(vk, startSlot, viewCount, bufferVector.data(), offsetVector.data());
        }
    }

    void VulkanCommandList::setViewports(const RenderViewport *viewports, uint32_t count) {
        if (count > 1) {
            thread_local std::vector<VkViewport> viewportVector;
            viewportVector.clear();

            for (uint32_t i = 0; i < count; i++) {
                viewportVector.emplace_back(VkViewport{ viewports[i].x, viewports[i].y, viewports[i].width, viewports[i].height, viewports[i].minDepth, viewports[i].maxDepth });
            }

            if (!viewportVector.empty()) {
                vkCmdSetViewport(vk, 0, uint32_t(viewportVector.size()), viewportVector.data());
            }
        }
        else {
            // Single element fast path.
            VkViewport viewport = VkViewport{ viewports[0].x, viewports[0].y, viewports[0].width, viewports[0].height, viewports[0].minDepth, viewports[0].maxDepth };
            vkCmdSetViewport(vk, 0, 1, &viewport);
        }
    }

    void VulkanCommandList::setScissors(const RenderRect *scissorRects, uint32_t count) {
        if (count > 1) {
            thread_local std::vector<VkRect2D> scissorVector;
            scissorVector.clear();

            for (uint32_t i = 0; i < count; i++) {
                scissorVector.emplace_back(VkRect2D{ VkOffset2D{ scissorRects[i].left, scissorRects[i].top }, VkExtent2D{ uint32_t(scissorRects[i].right - scissorRects[i].left), uint32_t(scissorRects[i].bottom - scissorRects[i].top) } });
            }

            if (!scissorVector.empty()) {
                vkCmdSetScissor(vk, 0, uint32_t(scissorVector.size()), scissorVector.data());
            }
        }
        else {
            // Single element fast path.
            VkRect2D scissor = VkRect2D{ VkOffset2D{ scissorRects[0].left, scissorRects[0].top }, VkExtent2D{ uint32_t(scissorRects[0].right - scissorRects[0].left), uint32_t(scissorRects[0].bottom - scissorRects[0].top) } };
            vkCmdSetScissor(vk, 0, 1, &scissor);
        }
    }

    void VulkanCommandList::setFramebuffer(const RenderFramebuffer *framebuffer) {
        endActiveRenderPass();

        if (framebuffer != nullptr) {
            const VulkanFramebuffer *interfaceFramebuffer = static_cast<const VulkanFramebuffer *>(framebuffer);
            targetFramebuffer = interfaceFramebuffer;
        }
        else {
            targetFramebuffer = nullptr;
        }
    }

    static void clearCommonRectVector(uint32_t width, uint32_t height, const RenderRect *clearRects, uint32_t clearRectsCount, std::vector<VkClearRect> &rectVector) {
        rectVector.clear();

        if (clearRectsCount > 0) {
            for (uint32_t i = 0; i < clearRectsCount; i++) {
                VkClearRect clearRect;
                clearRect.rect.offset.x = clearRects[i].left;
                clearRect.rect.offset.y = clearRects[i].top;
                clearRect.rect.extent.width = clearRects[i].right - clearRects[i].left;
                clearRect.rect.extent.height = clearRects[i].bottom - clearRects[i].top;
                clearRect.baseArrayLayer = 0;
                clearRect.layerCount = 1;
                rectVector.emplace_back(clearRect);
            }
        }
        else {
            VkClearRect clearRect;
            clearRect.rect.offset.x = 0;
            clearRect.rect.offset.y = 0;
            clearRect.rect.extent.width = width;
            clearRect.rect.extent.height = height;
            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = 1;
            rectVector.emplace_back(clearRect);
        }
    }

    void VulkanCommandList::clearColor(uint32_t attachmentIndex, RenderColor colorValue, const RenderRect *clearRects, uint32_t clearRectsCount) {
        assert(targetFramebuffer != nullptr);
        assert(attachmentIndex < targetFramebuffer->colorAttachments.size());
        assert((clearRectsCount == 0) || (clearRects != nullptr));

        checkActiveRenderPass();

        thread_local std::vector<VkClearRect> rectVector;
        clearCommonRectVector(targetFramebuffer->getWidth(), targetFramebuffer->getHeight(), clearRects, clearRectsCount, rectVector);

        VkClearAttachment attachment = {};
        auto &rgba = attachment.clearValue.color.float32;
        rgba[0] = colorValue.r;
        rgba[1] = colorValue.g;
        rgba[2] = colorValue.b;
        rgba[3] = colorValue.a;
        attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        attachment.colorAttachment = attachmentIndex;
        vkCmdClearAttachments(vk, 1, &attachment, uint32_t(rectVector.size()), rectVector.data());
    }

    void VulkanCommandList::clearDepth(bool clearDepth, float depthValue, const RenderRect *clearRects, uint32_t clearRectsCount) {
        assert(targetFramebuffer != nullptr);
        assert((clearRectsCount == 0) || (clearRects != nullptr));

        checkActiveRenderPass();

        thread_local std::vector<VkClearRect> rectVector;
        clearCommonRectVector(targetFramebuffer->getWidth(), targetFramebuffer->getHeight(), clearRects, clearRectsCount, rectVector);

        VkClearAttachment attachment = {};
        attachment.clearValue.depthStencil.depth = depthValue;

        if (clearDepth) {
            attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        vkCmdClearAttachments(vk, 1, &attachment, uint32_t(rectVector.size()), rectVector.data());
    }

    void VulkanCommandList::copyBufferRegion(RenderBufferReference dstBuffer, RenderBufferReference srcBuffer, uint64_t size) {
        endActiveRenderPass();

        assert(dstBuffer.ref != nullptr);
        assert(srcBuffer.ref != nullptr);

        const VulkanBuffer *interfaceDstBuffer = static_cast<const VulkanBuffer *>(dstBuffer.ref);
        const VulkanBuffer *interfaceSrcBuffer = static_cast<const VulkanBuffer *>(srcBuffer.ref);
        VkBufferCopy bufferCopy = {};
        bufferCopy.dstOffset = dstBuffer.offset;
        bufferCopy.srcOffset = srcBuffer.offset;
        bufferCopy.size = size;
        vkCmdCopyBuffer(vk, interfaceSrcBuffer->vk, interfaceDstBuffer->vk, 1, &bufferCopy);
    }

    void VulkanCommandList::copyTextureRegion(const RenderTextureCopyLocation &dstLocation, const RenderTextureCopyLocation &srcLocation, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const RenderBox *srcBox) {
        endActiveRenderPass();
        
        assert(dstLocation.type != RenderTextureCopyType::UNKNOWN);
        assert(srcLocation.type != RenderTextureCopyType::UNKNOWN);

        const VulkanTexture *dstTexture = static_cast<const VulkanTexture *>(dstLocation.texture);
        const VulkanTexture *srcTexture = static_cast<const VulkanTexture *>(srcLocation.texture);
        const VulkanBuffer *dstBuffer = static_cast<const VulkanBuffer *>(dstLocation.buffer);
        const VulkanBuffer *srcBuffer = static_cast<const VulkanBuffer *>(srcLocation.buffer);
        if ((dstLocation.type == RenderTextureCopyType::SUBRESOURCE) && (srcLocation.type == RenderTextureCopyType::PLACED_FOOTPRINT)) {
            assert(dstTexture != nullptr);
            assert(srcBuffer != nullptr);

            const uint32_t blockWidth = RenderFormatBlockWidth(dstTexture->desc.format);
            VkBufferImageCopy imageCopy = {};
            imageCopy.bufferOffset = srcLocation.placedFootprint.offset;
            imageCopy.bufferRowLength = ((srcLocation.placedFootprint.rowWidth + blockWidth - 1) / blockWidth) * blockWidth;
            imageCopy.bufferImageHeight = ((srcLocation.placedFootprint.height + blockWidth - 1) / blockWidth) * blockWidth;
            imageCopy.imageSubresource.aspectMask = (dstTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.imageSubresource.baseArrayLayer = dstLocation.subresource.index / dstTexture->desc.mipLevels;
            imageCopy.imageSubresource.layerCount = 1;
            imageCopy.imageSubresource.mipLevel = dstLocation.subresource.index % dstTexture->desc.mipLevels;
            imageCopy.imageOffset.x = dstX;
            imageCopy.imageOffset.y = dstY;
            imageCopy.imageOffset.z = dstZ;
            imageCopy.imageExtent.width = srcLocation.placedFootprint.width;
            imageCopy.imageExtent.height = srcLocation.placedFootprint.height;
            imageCopy.imageExtent.depth = srcLocation.placedFootprint.depth;
            vkCmdCopyBufferToImage(vk, srcBuffer->vk, dstTexture->vk, toImageLayout(dstTexture->textureLayout), 1, &imageCopy);
        }
        else {
            VkImageCopy imageCopy = {};
            imageCopy.srcSubresource.aspectMask = (srcTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.srcSubresource.baseArrayLayer = 0;
            imageCopy.srcSubresource.layerCount = 1;
            imageCopy.srcSubresource.mipLevel = srcLocation.subresource.index;
            imageCopy.dstSubresource.aspectMask = (dstTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageCopy.dstSubresource.baseArrayLayer = 0;
            imageCopy.dstSubresource.layerCount = 1;
            imageCopy.dstSubresource.mipLevel = dstLocation.subresource.index;
            imageCopy.dstOffset.x = dstX;
            imageCopy.dstOffset.y = dstY;
            imageCopy.dstOffset.z = dstZ;

            if (srcBox != nullptr) {
                imageCopy.srcOffset.x = srcBox->left;
                imageCopy.srcOffset.y = srcBox->top;
                imageCopy.srcOffset.z = srcBox->front;
                imageCopy.extent.width = srcBox->right - srcBox->left;
                imageCopy.extent.height = srcBox->bottom - srcBox->top;
                imageCopy.extent.depth = srcBox->back - srcBox->front;
            }
            else {
                imageCopy.srcOffset.x = 0;
                imageCopy.srcOffset.y = 0;
                imageCopy.srcOffset.z = 0;
                imageCopy.extent.width = srcTexture->desc.width;
                imageCopy.extent.height = srcTexture->desc.height;
                imageCopy.extent.depth = srcTexture->desc.depth;
            }

            vkCmdCopyImage(vk, srcTexture->vk, toImageLayout(srcTexture->textureLayout), dstTexture->vk, toImageLayout(dstTexture->textureLayout), 1, &imageCopy);
        }
    }

    void VulkanCommandList::copyBuffer(const RenderBuffer *dstBuffer, const RenderBuffer *srcBuffer) {
        endActiveRenderPass();

        assert(dstBuffer != nullptr);
        assert(srcBuffer != nullptr);
        
        const VulkanBuffer *interfaceDstBuffer = static_cast<const VulkanBuffer *>(dstBuffer);
        const VulkanBuffer *interfaceSrcBuffer = static_cast<const VulkanBuffer *>(srcBuffer);
        VkBufferCopy bufferCopy = {};
        bufferCopy.dstOffset = 0;
        bufferCopy.srcOffset = 0;
        bufferCopy.size = interfaceDstBuffer->desc.size;
        vkCmdCopyBuffer(vk, interfaceSrcBuffer->vk, interfaceDstBuffer->vk, 1, &bufferCopy);
    }

    void VulkanCommandList::copyTexture(const RenderTexture *dstTexture, const RenderTexture *srcTexture) {
        endActiveRenderPass();

        assert(dstTexture != nullptr);
        assert(srcTexture != nullptr);

        thread_local std::vector<VkImageCopy> imageCopies;
        imageCopies.clear();

        const VulkanTexture *dst = static_cast<const VulkanTexture *>(dstTexture);
        const VulkanTexture *src = static_cast<const VulkanTexture *>(srcTexture);
        VkImageLayout srcLayout = toImageLayout(src->textureLayout);
        VkImageLayout dstLayout = toImageLayout(dst->textureLayout);
        VkImageCopy imageCopy = {};
        imageCopy.srcSubresource.aspectMask = (src->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.srcSubresource.baseArrayLayer = 0;
        imageCopy.srcSubresource.layerCount = 1;
        imageCopy.dstSubresource.aspectMask = (dst->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.dstSubresource.baseArrayLayer = 0;
        imageCopy.dstSubresource.layerCount = 1;
        imageCopy.extent.width = uint32_t(dst->desc.width);
        imageCopy.extent.height = dst->desc.height;
        imageCopy.extent.depth = dst->desc.depth;

        assert(dst->desc.mipLevels > 0);
        assert(src->desc.mipLevels == dst->desc.mipLevels);

        for (uint32_t i = 0; i < dst->desc.mipLevels; i++) {
            imageCopy.srcSubresource.mipLevel = i;
            imageCopy.dstSubresource.mipLevel = i;
            imageCopies.emplace_back(imageCopy);
        }

        vkCmdCopyImage(vk, src->vk, srcLayout, dst->vk, dstLayout, uint32_t(imageCopies.size()), imageCopies.data());
    }

    void VulkanCommandList::resolveTexture(const RenderTexture *dstTexture, const RenderTexture *srcTexture) {
        resolveTextureRegion(dstTexture, 0, 0, srcTexture, nullptr);
    }

    void VulkanCommandList::resolveTextureRegion(const RenderTexture *dstTexture, uint32_t dstX, uint32_t dstY, const RenderTexture *srcTexture, const RenderRect *srcRect) {
        assert(dstTexture != nullptr);
        assert(srcTexture != nullptr);

        thread_local std::vector<VkImageResolve> imageResolves;
        imageResolves.clear();

        const VulkanTexture *dst = static_cast<const VulkanTexture *>(dstTexture);
        const VulkanTexture *src = static_cast<const VulkanTexture *>(srcTexture);
        VkImageLayout srcLayout = toImageLayout(src->textureLayout);
        VkImageLayout dstLayout = toImageLayout(dst->textureLayout);
        VkImageResolve imageResolve = {};
        imageResolve.srcSubresource.aspectMask = (src->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageResolve.srcSubresource.baseArrayLayer = 0;
        imageResolve.srcSubresource.layerCount = 1;
        imageResolve.dstOffset.x = dstX;
        imageResolve.dstOffset.y = dstY;
        imageResolve.dstSubresource.aspectMask = (dst->desc.flags & RenderTextureFlag::DEPTH_TARGET) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageResolve.dstSubresource.baseArrayLayer = 0;
        imageResolve.dstSubresource.layerCount = 1;
        imageResolve.extent.depth = dst->desc.depth;

        if (srcRect != nullptr) {
            imageResolve.srcOffset.x = srcRect->left;
            imageResolve.srcOffset.y = srcRect->top;
            imageResolve.extent.width = (srcRect->right - srcRect->left);
            imageResolve.extent.height = (srcRect->bottom - srcRect->top);
        }
        else {
            imageResolve.extent.width = uint32_t(dst->desc.width);
            imageResolve.extent.height = dst->desc.height;
        }

        assert(dst->desc.mipLevels > 0);
        assert(src->desc.mipLevels == dst->desc.mipLevels);

        for (uint32_t i = 0; i < dst->desc.mipLevels; i++) {
            imageResolve.srcSubresource.mipLevel = i;
            imageResolve.dstSubresource.mipLevel = i;
            imageResolves.emplace_back(imageResolve);
        }

        vkCmdResolveImage(vk, src->vk, srcLayout, dst->vk, dstLayout, uint32_t(imageResolves.size()), imageResolves.data());
    }
    
    void VulkanCommandList::buildBottomLevelAS(const RenderAccelerationStructure *dstAccelerationStructure, RenderBufferReference scratchBuffer, const RenderBottomLevelASBuildInfo &buildInfo) {
        assert(dstAccelerationStructure != nullptr);
        assert(scratchBuffer.ref != nullptr);

        const VulkanAccelerationStructure *interfaceAccelerationStructure = static_cast<const VulkanAccelerationStructure *>(dstAccelerationStructure);
        assert(interfaceAccelerationStructure->type == RenderAccelerationStructureType::BOTTOM_LEVEL);

        const VulkanBuffer *interfaceScratchBuffer = static_cast<const VulkanBuffer *>(scratchBuffer.ref);
        assert((interfaceScratchBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH) && "Scratch buffer must be allowed.");

        VkBufferDeviceAddressInfo scratchAddressInfo = {};
        scratchAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        scratchAddressInfo.buffer = interfaceScratchBuffer->vk;

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildGeometryInfo.flags = toRTASBuildFlags(buildInfo.preferFastBuild, buildInfo.preferFastTrace);
        buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfo.dstAccelerationStructure = interfaceAccelerationStructure->vk;
        buildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(device->vk, &scratchAddressInfo) + scratchBuffer.offset;
        buildGeometryInfo.pGeometries = reinterpret_cast<const VkAccelerationStructureGeometryKHR *>(buildInfo.buildData.data());
        buildGeometryInfo.geometryCount = buildInfo.meshCount;

        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {};
        buildRangeInfo.primitiveCount = buildInfo.primitiveCount;
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0;

        VkAccelerationStructureBuildRangeInfoKHR *buildRangeInfoPtr = &buildRangeInfo;
        vkCmdBuildAccelerationStructuresKHR(vk, 1, &buildGeometryInfo, &buildRangeInfoPtr);
    }

    void VulkanCommandList::buildTopLevelAS(const RenderAccelerationStructure *dstAccelerationStructure, RenderBufferReference scratchBuffer, RenderBufferReference instancesBuffer, const RenderTopLevelASBuildInfo &buildInfo) {
        assert(dstAccelerationStructure != nullptr);
        assert(scratchBuffer.ref != nullptr);
        assert(instancesBuffer.ref != nullptr);

        const VulkanAccelerationStructure *interfaceAccelerationStructure = static_cast<const VulkanAccelerationStructure *>(dstAccelerationStructure);
        assert(interfaceAccelerationStructure->type == RenderAccelerationStructureType::TOP_LEVEL);

        const VulkanBuffer *interfaceScratchBuffer = static_cast<const VulkanBuffer *>(scratchBuffer.ref);
        assert((interfaceScratchBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH) && "Scratch buffer must be allowed.");

        VkBufferDeviceAddressInfo scratchAddressInfo = {};
        scratchAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        scratchAddressInfo.buffer = interfaceScratchBuffer->vk;

        const VulkanBuffer *interfaceInstancesBuffer = static_cast<const VulkanBuffer *>(instancesBuffer.ref);
        assert((interfaceInstancesBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input must be allowed.");

        VkBufferDeviceAddressInfo instancesAddressInfo = {};
        instancesAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        instancesAddressInfo.buffer = interfaceInstancesBuffer->vk;

        VkAccelerationStructureGeometryKHR topGeometry = {};
        topGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        topGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureGeometryInstancesDataKHR &instancesData = topGeometry.geometry.instances;
        instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        instancesData.data.deviceAddress = vkGetBufferDeviceAddress(device->vk, &instancesAddressInfo) + instancesBuffer.offset;

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfo.dstAccelerationStructure = interfaceAccelerationStructure->vk;
        buildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(device->vk, &scratchAddressInfo) + scratchBuffer.offset;
        buildGeometryInfo.pGeometries = &topGeometry;
        buildGeometryInfo.geometryCount = 1;

        VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {};
        buildRangeInfo.primitiveCount = buildInfo.instanceCount;
        buildRangeInfo.primitiveOffset = 0;
        buildRangeInfo.firstVertex = 0;
        buildRangeInfo.transformOffset = 0;

        VkAccelerationStructureBuildRangeInfoKHR *buildRangeInfoPtr = &buildRangeInfo;
        vkCmdBuildAccelerationStructuresKHR(vk, 1, &buildGeometryInfo, &buildRangeInfoPtr);
    }

    void VulkanCommandList::discardTexture(const RenderTexture* texture) {
        // Not required in Vulkan.
    }

    void VulkanCommandList::checkActiveRenderPass() {
        assert(targetFramebuffer != nullptr);
        
        if (activeRenderPass == VK_NULL_HANDLE) {
            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.renderPass = targetFramebuffer->renderPass;
            beginInfo.framebuffer = targetFramebuffer->vk;
            beginInfo.renderArea.extent.width = targetFramebuffer->width;
            beginInfo.renderArea.extent.height = targetFramebuffer->height;
            vkCmdBeginRenderPass(vk, &beginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
            activeRenderPass = targetFramebuffer->renderPass;
        }
    }

    void VulkanCommandList::endActiveRenderPass() {
        if (activeRenderPass != VK_NULL_HANDLE) {
            vkCmdEndRenderPass(vk);
            activeRenderPass = VK_NULL_HANDLE;
        }
    }

    void VulkanCommandList::setDescriptorSet(VkPipelineBindPoint bindPoint, const VulkanPipelineLayout *pipelineLayout, const RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        assert(pipelineLayout != nullptr);
        assert(descriptorSet != nullptr);
        assert(setIndex < pipelineLayout->descriptorSetLayouts.size());

        const VulkanDescriptorSet *interfaceSet = static_cast<const VulkanDescriptorSet *>(descriptorSet);
        vkCmdBindDescriptorSets(vk, bindPoint, pipelineLayout->vk, setIndex, 1, &interfaceSet->vk, 0, nullptr);
    }

    // VulkanCommandFence

    VulkanCommandFence::VulkanCommandFence(VulkanDevice *device) {
        assert(device != nullptr);

        this->device = device;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkResult res = vkCreateFence(device->vk, &fenceInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateFence failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanCommandFence::~VulkanCommandFence() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroyFence(device->vk, vk, nullptr);
        }
    }

    // VulkanCommandSemaphore

    VulkanCommandSemaphore::VulkanCommandSemaphore(VulkanDevice *device) {
        assert(device != nullptr);

        this->device = device;

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkResult res = vkCreateSemaphore(device->vk, &semaphoreInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateSemaphore failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanCommandSemaphore::~VulkanCommandSemaphore() {
        if (vk != VK_NULL_HANDLE) {
            vkDestroySemaphore(device->vk, vk, nullptr);
        }
    }

    // VulkanCommandQueue

    VulkanCommandQueue::VulkanCommandQueue(VulkanDevice *device, RenderCommandListType commandListType) {
        assert(device != nullptr);
        assert(commandListType != RenderCommandListType::UNKNOWN);

        this->device = device;

        familyIndex = device->queueFamilyIndices[toFamilyIndex(commandListType)];
        device->queueFamilies[familyIndex].add(this);
    }

    VulkanCommandQueue::~VulkanCommandQueue() {
        device->queueFamilies[familyIndex].remove(this);
    }

    std::unique_ptr<RenderSwapChain> VulkanCommandQueue::createSwapChain(RenderWindow renderWindow, uint32_t bufferCount, RenderFormat format) {
        return std::make_unique<VulkanSwapChain>(this, renderWindow, bufferCount, format);
    }

    void VulkanCommandQueue::executeCommandLists(const RenderCommandList **commandLists, uint32_t commandListCount, RenderCommandSemaphore **waitSemaphores, uint32_t waitSemaphoreCount, RenderCommandSemaphore **signalSemaphores, uint32_t signalSemaphoreCount, RenderCommandFence *signalFence) {
        assert(commandLists != nullptr);
        assert(commandListCount > 0);

        thread_local std::vector<VkSemaphore> waitSemaphoreVector;
        thread_local std::vector<VkSemaphore> signalSemaphoreVector;
        thread_local std::vector<VkCommandBuffer> commandBuffers;
        waitSemaphoreVector.clear();
        signalSemaphoreVector.clear();
        commandBuffers.clear();

        for (uint32_t i = 0; i < waitSemaphoreCount; i++) {
            VulkanCommandSemaphore *interfaceSemaphore = static_cast<VulkanCommandSemaphore *>(waitSemaphores[i]);
            waitSemaphoreVector.emplace_back(interfaceSemaphore->vk);
        }

        for (uint32_t i = 0; i < signalSemaphoreCount; i++) {
            VulkanCommandSemaphore *interfaceSemaphore = static_cast<VulkanCommandSemaphore *>(signalSemaphores[i]);
            signalSemaphoreVector.emplace_back(interfaceSemaphore->vk);
        }

        for (uint32_t i = 0; i < commandListCount; i++) {
            assert(commandLists[i] != nullptr);

            const VulkanCommandList *interfaceCommandList = static_cast<const VulkanCommandList *>(commandLists[i]);
            commandBuffers.emplace_back(interfaceCommandList->vk);
        }

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pCommandBuffers = commandBuffers.data();
        submitInfo.commandBufferCount = uint32_t(commandBuffers.size());

        const VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        if (!waitSemaphoreVector.empty()) {
            submitInfo.pWaitSemaphores = waitSemaphoreVector.data();
            submitInfo.waitSemaphoreCount = uint32_t(waitSemaphoreVector.size());
            submitInfo.pWaitDstStageMask = &waitStages;
        }

        if (!signalSemaphoreVector.empty()) {
            submitInfo.pSignalSemaphores = signalSemaphoreVector.data();
            submitInfo.signalSemaphoreCount = uint32_t(signalSemaphoreVector.size());
        }

        VkFence submitFence = VK_NULL_HANDLE;
        if (signalFence != nullptr) {
            VulkanCommandFence *interfaceFence = static_cast<VulkanCommandFence *>(signalFence);
            submitFence = interfaceFence->vk;
        }

        VkResult res;
        {
            const std::scoped_lock queueLock(*queue->mutex);
            res = vkQueueSubmit(queue->vk, 1, &submitInfo, submitFence);
        }

        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkQueueSubmit failed with error code 0x%X.\n", res);
            return;
        }
    }
    
    void VulkanCommandQueue::waitForCommandFence(RenderCommandFence *fence) {
        assert(fence != nullptr);

        VulkanCommandFence *interfaceFence = static_cast<VulkanCommandFence *>(fence);
        VkResult res = vkWaitForFences(device->vk, 1, &interfaceFence->vk, VK_TRUE, UINT64_MAX);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkWaitForFences failed with error code 0x%X.\n", res);
            return;
        }

        vkResetFences(device->vk, 1, &interfaceFence->vk);
    }

    // VulkanPool

    VulkanPool::VulkanPool(VulkanDevice *device, const RenderPoolDesc &desc) {
        assert(device != nullptr);

        this->device = device;

        VmaAllocationCreateInfo memoryInfo = {};
        switch (desc.heapType) {
        case RenderHeapType::DEFAULT:
            memoryInfo.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case RenderHeapType::UPLOAD:
            memoryInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            break;
        case RenderHeapType::READBACK:
            memoryInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
            break;
        default:
            assert(false && "Unknown heap type.");
            break;
        }

        uint32_t memoryTypeIndex = 0;
        VkResult res = vmaFindMemoryTypeIndex(device->allocator, UINT32_MAX, &memoryInfo, &memoryTypeIndex);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vmaFindMemoryTypeIndex failed with error code 0x%X.\n", res);
            return;
        }

        VmaPoolCreateInfo createInfo = {};
        createInfo.memoryTypeIndex = memoryTypeIndex;
        createInfo.minBlockCount = desc.minBlockCount;
        createInfo.maxBlockCount = desc.maxBlockCount;
        createInfo.flags |= desc.useLinearAlgorithm ? VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT : 0;

        res = vmaCreatePool(device->allocator, &createInfo, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vmaCreatePool failed with error code 0x%X.\n", res);
            return;
        }
    }

    VulkanPool::~VulkanPool() {
        if (vk != VK_NULL_HANDLE) {
            vmaDestroyPool(device->allocator, vk);
        }
    }

    std::unique_ptr<RenderBuffer> VulkanPool::createBuffer(const RenderBufferDesc &desc) {
        return std::make_unique<VulkanBuffer>(device, this, desc);
    }

    std::unique_ptr<RenderTexture> VulkanPool::createTexture(const RenderTextureDesc &desc) {
        return std::make_unique<VulkanTexture>(device, this, desc);
    }
    
    // VulkanQueueFamily

    void VulkanQueueFamily::add(VulkanCommandQueue *virtualQueue) {
        assert(virtualQueue != nullptr);

        // Insert virtual queue into the queue with the least amount of virtual queues.
        uint32_t queueIndex = 0;
        uint32_t lowestCount = UINT_MAX;
        for (uint32_t i = 0; i < queues.size(); i++) {
            uint32_t virtualQueueCount = uint32_t(queues[i].virtualQueues.size());
            if (virtualQueueCount < lowestCount) {
                queueIndex = i;
                lowestCount = virtualQueueCount;
            }
        }

        if (queues[queueIndex].mutex == nullptr) {
            queues[queueIndex].mutex = std::make_unique<std::mutex>();
        }

        virtualQueue->queue = &queues[queueIndex];
        virtualQueue->queueIndex = queueIndex;
        queues[queueIndex].virtualQueues.insert(virtualQueue);
    }

    void VulkanQueueFamily::remove(VulkanCommandQueue *virtualQueue) {
        assert(virtualQueue != nullptr);

        queues[virtualQueue->queueIndex].virtualQueues.erase(virtualQueue);
    }

    // VulkanDevice
    
    VulkanDevice::VulkanDevice(VulkanInterface *renderInterface) {
        assert(renderInterface != nullptr);

        this->renderInterface = renderInterface;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(renderInterface->instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            fprintf(stderr, "Unable to find devices that support Vulkan.\n");
            return;
        }
        
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(renderInterface->instance, &deviceCount, physicalDevices.data());

        uint32_t currentDeviceTypeScore = 0;
        uint32_t deviceTypeScoreTable[] = {
            0,  // VK_PHYSICAL_DEVICE_TYPE_OTHER
            3,  // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
            4,  // VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            2,  // VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU
            1   // VK_PHYSICAL_DEVICE_TYPE_CPU
        };

        for (uint32_t i = 0; i < deviceCount; i++) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

            uint32_t deviceTypeIndex = deviceProperties.deviceType;
            if (deviceTypeIndex > 4) {
                continue;
            }

            uint32_t deviceTypeScore = deviceTypeScoreTable[deviceTypeIndex];
            bool preferDeviceTypeScore = (deviceTypeScore > currentDeviceTypeScore);
            bool preferOption = preferDeviceTypeScore;
            if (preferOption) {
                physicalDevice = physicalDevices[i];
                description.name = std::string(deviceProperties.deviceName);
                description.driverVersion = deviceProperties.driverVersion;
                currentDeviceTypeScore = deviceTypeScore;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            fprintf(stderr, "Unable to find a device with the required features.\n");
            return;
        }

        // Check for extensions.
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> missingRequiredExtensions = RequiredDeviceExtensions;
        std::unordered_set<std::string> supportedOptionalExtensions;
#   if DLSS_ENABLED
        const std::unordered_set<std::string> dlssExtensions = DLSS::getRequiredDeviceExtensionsVulkan(this);
#   endif
        for (uint32_t i = 0; i < extensionCount; i++) {
            const std::string extensionName(availableExtensions[i].extensionName);
            missingRequiredExtensions.erase(extensionName);

            if (OptionalDeviceExtensions.find(extensionName) != OptionalDeviceExtensions.end()) {
                supportedOptionalExtensions.insert(extensionName);
            }
#       if DLSS_ENABLED
            else if (dlssExtensions.find(extensionName) != dlssExtensions.end()) {
                supportedOptionalExtensions.insert(extensionName);
            }
#       endif
        }
        
        if (!missingRequiredExtensions.empty()) {
            for (const std::string &extension : missingRequiredExtensions) {
                fprintf(stderr, "Missing required extension: %s.\n", extension.c_str());
            }

            fprintf(stderr, "Unable to create device. Required extensions are missing.\n");
            return;
        }

        // Store properties.
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        // Check for supported features.
        void *featuresChain = nullptr;
        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = {};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        featuresChain = &indexingFeatures;

        VkPhysicalDeviceScalarBlockLayoutFeatures layoutFeatures = {};
        layoutFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
        layoutFeatures.pNext = featuresChain;
        featuresChain = &layoutFeatures;

        VkPhysicalDevicePresentIdFeaturesKHR presentIdFeatures = {};
        VkPhysicalDevicePresentWaitFeaturesKHR presentWaitFeatures = {};
        const bool presentWaitSupported = supportedOptionalExtensions.find(VK_KHR_PRESENT_ID_EXTENSION_NAME) != supportedOptionalExtensions.end() && supportedOptionalExtensions.find(VK_KHR_PRESENT_WAIT_EXTENSION_NAME) != supportedOptionalExtensions.end();
        if (presentWaitSupported) {
            presentIdFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;
            presentIdFeatures.pNext = featuresChain;
            featuresChain = &presentIdFeatures;

            presentWaitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR;
            presentWaitFeatures.pNext = featuresChain;
            featuresChain = &presentWaitFeatures;
        }

        VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures = {};
        robustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
        robustnessFeatures.pNext = featuresChain;
        featuresChain = &robustnessFeatures;

        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
        bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        bufferDeviceAddressFeatures.pNext = featuresChain;
        featuresChain = &bufferDeviceAddressFeatures;

        VkPhysicalDeviceFeatures2 deviceFeatures = {};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = featuresChain;
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures);

        void *createDeviceChain = nullptr;
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {};
        const bool rtSupported = supportedOptionalExtensions.find(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) != supportedOptionalExtensions.end();
        if (rtSupported) {
            rtPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

            VkPhysicalDeviceProperties2 deviceProperties2 = {};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rtPipelineProperties;
            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

            rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
            rtPipelineFeatures.rayTracingPipeline = true;
            createDeviceChain = &rtPipelineFeatures;

            accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
            accelerationStructureFeatures.pNext = createDeviceChain;
            accelerationStructureFeatures.accelerationStructure = true;
            createDeviceChain = &accelerationStructureFeatures;
        }
        
        const bool sampleLocationsSupported = supportedOptionalExtensions.find(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME) != supportedOptionalExtensions.end();
        if (sampleLocationsSupported) {
            sampleLocationProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;

            VkPhysicalDeviceProperties2 deviceProperties2 = {};
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &sampleLocationProperties;
            vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);
        }

        const bool descriptorIndexing = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.descriptorBindingVariableDescriptorCount && indexingFeatures.runtimeDescriptorArray;
        if (descriptorIndexing) {
            indexingFeatures.pNext = createDeviceChain;
            createDeviceChain = &indexingFeatures;
        }

        const bool scalarBlockLayout = layoutFeatures.scalarBlockLayout;
        if (scalarBlockLayout) {
            layoutFeatures.pNext = createDeviceChain;
            createDeviceChain = &layoutFeatures;
        }

        const bool presentWait = presentIdFeatures.presentId && presentWaitFeatures.presentWait;
        if (presentWait) {
            presentIdFeatures.pNext = createDeviceChain;
            createDeviceChain = &presentIdFeatures;

            presentWaitFeatures.pNext = createDeviceChain;
            createDeviceChain = &presentWaitFeatures;
        }

        const bool nullDescriptor = robustnessFeatures.nullDescriptor;
        if (nullDescriptor) {
            robustnessFeatures.pNext = createDeviceChain;
            createDeviceChain = &robustnessFeatures;
        }

        const bool bufferDeviceAddress = bufferDeviceAddressFeatures.bufferDeviceAddress;
        if (bufferDeviceAddress) {
            bufferDeviceAddressFeatures.pNext = createDeviceChain;
            createDeviceChain = &bufferDeviceAddressFeatures;
        }

        // Retrieve the information for the queue families.
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        std::vector<bool> queueFamilyUsed(queueFamilyCount, false);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

        auto pickFamilyQueue = [&](RenderCommandListType type, VkQueueFlags flags) {
            uint32_t familyIndex = 0;
            uint32_t familySetBits = sizeof(uint32_t) * 8;
            uint32_t familyQueueCount = 0;
            for (uint32_t i = 0; i < queueFamilyCount; i++) {
                const VkQueueFamilyProperties &props = queueFamilyProperties[i];

                // The family queue flags must contain all the flags required by the command list type.
                if ((props.queueFlags & flags) != flags) {
                    continue;
                }

                // Prefer picking the queues with the least amount of bits set that match the mask we're looking for.
                uint32_t setBits = numberOfSetBits(props.queueFlags);
                if ((setBits < familySetBits) || ((setBits == familySetBits) && (props.queueCount > familyQueueCount))) {
                    familyIndex = i;
                    familySetBits = setBits;
                    familyQueueCount = props.queueCount;
                }
            }

            queueFamilyIndices[toFamilyIndex(type)] = familyIndex;
            queueFamilyUsed[familyIndex] = true;
        };

        // Pick the family queues for each type of command list.
        pickFamilyQueue(RenderCommandListType::DIRECT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
        pickFamilyQueue(RenderCommandListType::COMPUTE, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
        pickFamilyQueue(RenderCommandListType::COPY, VK_QUEUE_TRANSFER_BIT);

        // Create the struct to store the virtual queues.
        queueFamilies.resize(queueFamilyCount);

        // Create the logical device with the desired family queues.
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::vector<float> queuePriorities(MaxQueuesPerFamilyCount, 1.0f);
        queueCreateInfos.reserve(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilyUsed[i]) {
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueCount = std::min(queueFamilyProperties[i].queueCount, MaxQueuesPerFamilyCount);
                queueCreateInfo.queueFamilyIndex = i;
                queueCreateInfo.pQueuePriorities = queuePriorities.data();
                queueCreateInfos.emplace_back(queueCreateInfo);
                queueFamilies[i].queues.resize(queueCreateInfo.queueCount);
            }
        }

        std::vector<const char *> enabledExtensions;
        for (const std::string &extension : RequiredDeviceExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        for (const std::string &extension : supportedOptionalExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = createDeviceChain;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = uint32_t(queueCreateInfos.size());
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        createInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());
        createInfo.pEnabledFeatures = &deviceFeatures.features;

        VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &vk);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateDevice failed with error code 0x%X.\n", res);
            return;
        }

        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            for (uint32_t j = 0; j < queueFamilies[i].queues.size(); j++) {
                vkGetDeviceQueue(vk, i, j, &queueFamilies[i].queues[j].vk);
            }
        }

        VmaVulkanFunctions vmaFunctions = {};
        vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
        vmaFunctions.vkAllocateMemory = vkAllocateMemory;
        vmaFunctions.vkBindBufferMemory = vkBindBufferMemory;
        vmaFunctions.vkBindImageMemory = vkBindImageMemory;
        vmaFunctions.vkCreateBuffer = vkCreateBuffer;
        vmaFunctions.vkCreateImage = vkCreateImage;
        vmaFunctions.vkDestroyBuffer = vkDestroyBuffer;
        vmaFunctions.vkDestroyImage = vkDestroyImage;
        vmaFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
        vmaFunctions.vkFreeMemory = vkFreeMemory;
        vmaFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
        vmaFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
        vmaFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
        vmaFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
        vmaFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
        vmaFunctions.vkMapMemory = vkMapMemory;
        vmaFunctions.vkUnmapMemory = vkUnmapMemory;
        vmaFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.flags |= bufferDeviceAddress ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = vk;
        allocatorInfo.pVulkanFunctions = &vmaFunctions;
        allocatorInfo.instance = renderInterface->instance;
        allocatorInfo.vulkanApiVersion = renderInterface->appInfo.apiVersion;

        res = vmaCreateAllocator(&allocatorInfo, &allocator);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vmaCreateAllocator failed with error code 0x%X.\n", res);
            release();
            return;
        }

        // Find the biggest device local memory available on the device.
        VkDeviceSize memoryHeapSize = 0;
        const VkPhysicalDeviceMemoryProperties *memoryProps = nullptr;
        vmaGetMemoryProperties(allocator, &memoryProps);
        for (uint32_t i = 0; i < memoryProps->memoryHeapCount; i++) {
            if (memoryProps->memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                memoryHeapSize = std::max(memoryProps->memoryHeaps[i].size, memoryHeapSize);
            }
        }

        // Fill description.
        description.dedicatedVideoMemory = memoryHeapSize;

        // Fill capabilities.
        capabilities.raytracing = rtSupported;
        capabilities.raytracingStateUpdate = false;
        capabilities.sampleLocations = sampleLocationsSupported;
        capabilities.descriptorIndexing = descriptorIndexing;
        capabilities.scalarBlockLayout = scalarBlockLayout;
        capabilities.presentWait = presentWait;
        capabilities.displayTiming = supportedOptionalExtensions.find(VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME) != supportedOptionalExtensions.end();
        capabilities.preferHDR = memoryHeapSize > (512 * 1024 * 1024);

        // Fill Vulkan-only capabilities.
        loadStoreOpNoneSupported = supportedOptionalExtensions.find(VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME) != supportedOptionalExtensions.end();
    }

    VulkanDevice::~VulkanDevice() {
        release();
    }

    std::unique_ptr<RenderCommandList> VulkanDevice::createCommandList(RenderCommandListType type) {
        return std::make_unique<VulkanCommandList>(this, type);
    }

    std::unique_ptr<RenderDescriptorSet> VulkanDevice::createDescriptorSet(const RenderDescriptorSetDesc &desc) {
        return std::make_unique<VulkanDescriptorSet>(this, desc);
    }

    std::unique_ptr<RenderShader> VulkanDevice::createShader(const void *data, uint64_t size, const char *entryPointName, RenderShaderFormat format) {
        return std::make_unique<VulkanShader>(this, data, size, entryPointName, format);
    }

    std::unique_ptr<RenderSampler> VulkanDevice::createSampler(const RenderSamplerDesc &desc) {
        return std::make_unique<VulkanSampler>(this, desc);
    }

    std::unique_ptr<RenderPipeline> VulkanDevice::createComputePipeline(const RenderComputePipelineDesc &desc) {
        return std::make_unique<VulkanComputePipeline>(this, desc);
    }

    std::unique_ptr<RenderPipeline> VulkanDevice::createGraphicsPipeline(const RenderGraphicsPipelineDesc &desc) {
        return std::make_unique<VulkanGraphicsPipeline>(this, desc);
    }

    std::unique_ptr<RenderPipeline> VulkanDevice::createRaytracingPipeline(const RenderRaytracingPipelineDesc &desc, const RenderPipeline *previousPipeline) {
        return std::make_unique<VulkanRaytracingPipeline>(this, desc, previousPipeline);
    }

    std::unique_ptr<RenderCommandQueue> VulkanDevice::createCommandQueue(RenderCommandListType type) {
        return std::make_unique<VulkanCommandQueue>(this, type);
    }

    std::unique_ptr<RenderBuffer> VulkanDevice::createBuffer(const RenderBufferDesc &desc) {
        return std::make_unique<VulkanBuffer>(this, nullptr, desc);
    }

    std::unique_ptr<RenderTexture> VulkanDevice::createTexture(const RenderTextureDesc &desc) {
        return std::make_unique<VulkanTexture>(this, nullptr, desc);
    }

    std::unique_ptr<RenderAccelerationStructure> VulkanDevice::createAccelerationStructure(const RenderAccelerationStructureDesc &desc) {
        return std::make_unique<VulkanAccelerationStructure>(this, desc);
    }

    std::unique_ptr<RenderPool> VulkanDevice::createPool(const RenderPoolDesc &desc) {
        return std::make_unique<VulkanPool>(this, desc);
    }

    std::unique_ptr<RenderPipelineLayout> VulkanDevice::createPipelineLayout(const RenderPipelineLayoutDesc &desc) {
        return std::make_unique<VulkanPipelineLayout>(this, desc);
    }

    std::unique_ptr<RenderCommandFence> VulkanDevice::createCommandFence() {
        return std::make_unique<VulkanCommandFence>(this);
    }

    std::unique_ptr<RenderCommandSemaphore> VulkanDevice::createCommandSemaphore() {
        return std::make_unique<VulkanCommandSemaphore>(this);
    }

    std::unique_ptr<RenderFramebuffer> VulkanDevice::createFramebuffer(const RenderFramebufferDesc &desc) {
        return std::make_unique<VulkanFramebuffer>(this, desc);
    }

    void VulkanDevice::setBottomLevelASBuildInfo(RenderBottomLevelASBuildInfo &buildInfo, const RenderBottomLevelASMesh *meshes, uint32_t meshCount, bool preferFastBuild, bool preferFastTrace) {
        assert(meshes != nullptr);
        assert(meshCount > 0);

        uint32_t primitiveCount = 0;
        thread_local std::vector<uint32_t> geometryPrimitiveCounts;
        geometryPrimitiveCounts.resize(meshCount);

        buildInfo.buildData.resize(sizeof(VkAccelerationStructureGeometryKHR) * meshCount);
        VkAccelerationStructureGeometryKHR *geometries = reinterpret_cast<VkAccelerationStructureGeometryKHR *>(buildInfo.buildData.data());
        for (uint32_t i = 0; i < meshCount; i++) {
            const RenderBottomLevelASMesh &mesh = meshes[i];
            VkAccelerationStructureGeometryKHR &geometry = geometries[i];
            geometry = {};
            geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            geometry.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
            geometry.flags |= mesh.isOpaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;

            const VulkanBuffer *interfaceVertexBuffer = static_cast<const VulkanBuffer *>(mesh.vertexBuffer.ref);
            const VulkanBuffer *interfaceIndexBuffer = static_cast<const VulkanBuffer *>(mesh.indexBuffer.ref);
            assert((interfaceIndexBuffer == nullptr) || ((interfaceIndexBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input allowed on index buffer."));
            assert((interfaceVertexBuffer == nullptr) || ((interfaceVertexBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input allowed on vertex buffer."));

            VkAccelerationStructureGeometryTrianglesDataKHR &triangles = geometry.geometry.triangles;
            triangles = {};
            triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            triangles.vertexFormat = toVk(mesh.vertexFormat);
            triangles.vertexStride = mesh.vertexStride;
            triangles.maxVertex = mesh.vertexCount - 1;

            if (interfaceVertexBuffer != nullptr) {
                VkBufferDeviceAddressInfo vertexAddressInfo = {};
                vertexAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                vertexAddressInfo.buffer = interfaceVertexBuffer->vk;
                triangles.vertexData.deviceAddress = vkGetBufferDeviceAddress(vk, &vertexAddressInfo) + mesh.vertexBuffer.offset;
            }

            if (interfaceIndexBuffer != nullptr) {
                triangles.indexType = toIndexType(mesh.indexFormat);

                VkBufferDeviceAddressInfo indexAddressInfo = {};
                indexAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
                indexAddressInfo.buffer = interfaceIndexBuffer->vk;
                triangles.indexData.deviceAddress = vkGetBufferDeviceAddress(vk, &indexAddressInfo) + mesh.indexBuffer.offset;
                geometryPrimitiveCounts[i] = mesh.indexCount / 3;
            }
            else {
                triangles.indexType = VK_INDEX_TYPE_NONE_KHR;
                geometryPrimitiveCounts[i] = mesh.vertexCount / 3;
            }

            primitiveCount += geometryPrimitiveCounts[i];
        }

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildGeometryInfo.flags = toRTASBuildFlags(preferFastBuild, preferFastTrace);
        buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfo.pGeometries = geometries;
        buildGeometryInfo.geometryCount = meshCount;

        VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
        buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(vk, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, geometryPrimitiveCounts.data(), &buildSizesInfo);

        buildInfo.meshCount = meshCount;
        buildInfo.primitiveCount = primitiveCount;
        buildInfo.preferFastBuild = preferFastBuild;
        buildInfo.preferFastTrace = preferFastTrace;
        buildInfo.scratchSize = roundUp(buildSizesInfo.buildScratchSize, AccelerationStructureBufferAlignment);
        buildInfo.accelerationStructureSize = roundUp(buildSizesInfo.accelerationStructureSize, AccelerationStructureBufferAlignment);
    }

    void VulkanDevice::setTopLevelASBuildInfo(RenderTopLevelASBuildInfo &buildInfo, const RenderTopLevelASInstance *instances, uint32_t instanceCount, bool preferFastBuild, bool preferFastTrace) {
        assert(instances != nullptr);
        assert(instanceCount > 0);

        // Build the instance data to be uploaded.
        buildInfo.instancesBufferData.resize(sizeof(VkAccelerationStructureInstanceKHR) * instanceCount, 0);
        VkAccelerationStructureInstanceKHR *bufferInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR *>(buildInfo.instancesBufferData.data());
        for (uint32_t i = 0; i < instanceCount; i++) {
            const RenderTopLevelASInstance &instance = instances[i];
            const VulkanBuffer *interfaceBottomLevelAS = static_cast<const VulkanBuffer *>(instance.bottomLevelAS.ref);
            assert(interfaceBottomLevelAS != nullptr);

            VkAccelerationStructureInstanceKHR &bufferInstance = bufferInstances[i];
            bufferInstance.instanceCustomIndex = instance.instanceID;
            bufferInstance.mask = instance.instanceMask;
            bufferInstance.instanceShaderBindingTableRecordOffset = instance.instanceContributionToHitGroupIndex;
            bufferInstance.flags = instance.cullDisable ? VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR : 0;
            memcpy(bufferInstance.transform.matrix, instance.transform.m, sizeof(bufferInstance.transform.matrix));

            VkBufferDeviceAddressInfo blasAddressInfo = {};
            blasAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            blasAddressInfo.buffer = interfaceBottomLevelAS->vk;
            bufferInstance.accelerationStructureReference = vkGetBufferDeviceAddress(vk, &blasAddressInfo) + instance.bottomLevelAS.offset;
        }

        // Retrieve the size the TLAS will require.
        VkAccelerationStructureGeometryKHR topGeometry = {};
        topGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        topGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;

        VkAccelerationStructureGeometryInstancesDataKHR &instancesData = topGeometry.geometry.instances;
        instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;

        VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
        buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        buildGeometryInfo.flags = toRTASBuildFlags(preferFastBuild, preferFastTrace);
        buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfo.pGeometries = &topGeometry;
        buildGeometryInfo.geometryCount = 1;

        VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
        buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(vk, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &instanceCount, &buildSizesInfo);

        buildInfo.instanceCount = instanceCount;
        buildInfo.preferFastBuild = preferFastBuild;
        buildInfo.preferFastTrace = preferFastTrace;
        buildInfo.scratchSize = roundUp(buildSizesInfo.buildScratchSize, AccelerationStructureBufferAlignment);
        buildInfo.accelerationStructureSize = roundUp(buildSizesInfo.accelerationStructureSize, AccelerationStructureBufferAlignment);
    }
    
    void VulkanDevice::setShaderBindingTableInfo(RenderShaderBindingTableInfo &tableInfo, const RenderShaderBindingGroups &groups, const RenderPipeline *pipeline, RenderDescriptorSet **descriptorSets, uint32_t descriptorSetCount) {
        assert(pipeline != nullptr);
        assert((descriptorSets != nullptr) && "Vulkan doesn't require descriptor sets, but they should be passed to keep consistency with D3D12.");

        const VulkanRaytracingPipeline *raytracingPipeline = static_cast<const VulkanRaytracingPipeline *>(pipeline);
        assert((raytracingPipeline->type == VulkanPipeline::Type::Raytracing) && "Only raytracing pipelines can be used to build shader binding tables.");
        assert((raytracingPipeline->descriptorSetCount <= descriptorSetCount) && "There must be enough descriptor sets available for the pipeline.");

        const uint32_t handleSize = rtPipelineProperties.shaderGroupHandleSize;
        thread_local std::vector<uint8_t> groupHandles;
        groupHandles.clear();
        groupHandles.resize(raytracingPipeline->groupCount * handleSize, 0);
        VkResult res = vkGetRayTracingShaderGroupHandlesKHR(vk, raytracingPipeline->vk, 0, raytracingPipeline->groupCount, groupHandles.size(), groupHandles.data());
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkGetRayTracingShaderGroupHandlesKHR failed with error code 0x%X.\n", res);
            return;
        }
        
        const uint32_t handleSizeAligned = roundUp(handleSize, rtPipelineProperties.shaderGroupHandleAlignment);
        const uint32_t regionAlignment = roundUp(handleSizeAligned, rtPipelineProperties.shaderGroupBaseAlignment);
        uint64_t tableSize = 0;

        auto setGroup = [&](RenderShaderBindingGroupInfo &groupInfo, const RenderShaderBindingGroup &renderGroup) {
            groupInfo.startIndex = 0;

            if (renderGroup.pipelineProgramsCount == 0) {
                groupInfo.stride = 0;
                groupInfo.offset = 0;
                groupInfo.size = 0;
            }
            else {
                groupInfo.stride = regionAlignment;
                groupInfo.offset = tableSize;
                groupInfo.size = groupInfo.stride * renderGroup.pipelineProgramsCount;
                tableSize += groupInfo.size;
            }
        };

        setGroup(tableInfo.groups.rayGen, groups.rayGen);
        setGroup(tableInfo.groups.miss, groups.miss);
        setGroup(tableInfo.groups.hitGroup, groups.hitGroup);
        setGroup(tableInfo.groups.callable, groups.callable);

        tableSize = roundUp(tableSize, ShaderBindingTableAlignment);
        tableInfo.tableBufferData.clear();
        tableInfo.tableBufferData.resize(tableSize, 0);

        auto copyGroupData = [&](RenderShaderBindingGroupInfo &groupInfo, const RenderShaderBindingGroup &renderGroup) {
            for (uint32_t i = 0; i < renderGroup.pipelineProgramsCount; i++) {
                const uint8_t *shaderId = groupHandles.data() + renderGroup.pipelinePrograms[i].programIndex * handleSize;
                const uint64_t tableOffset = groupInfo.offset + i * groupInfo.stride;
                memcpy(&tableInfo.tableBufferData[tableOffset], shaderId, handleSize);
            }
        };

        copyGroupData(tableInfo.groups.rayGen, groups.rayGen);
        copyGroupData(tableInfo.groups.miss, groups.miss);
        copyGroupData(tableInfo.groups.hitGroup, groups.hitGroup);
        copyGroupData(tableInfo.groups.callable, groups.callable);
    }

    const RenderDeviceCapabilities &VulkanDevice::getCapabilities() const {
        return capabilities;
    }

    const RenderDeviceDescription &VulkanDevice::getDescription() const {
        return description;
    }
    
    RenderSampleCounts VulkanDevice::getSampleCountsSupported(RenderFormat format) const {
        const bool isDepthFormat = (format == RenderFormat::D16_UNORM) || (format == RenderFormat::D32_FLOAT);
        if (isDepthFormat) {
            return RenderSampleCounts(physicalDeviceProperties.limits.framebufferDepthSampleCounts);
        }
        else {
            return RenderSampleCounts(physicalDeviceProperties.limits.framebufferColorSampleCounts);
        }
    }

    void VulkanDevice::waitIdle() const {
        vkDeviceWaitIdle(vk);
    }

    void VulkanDevice::release() {
        if (allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }

        if (vk != VK_NULL_HANDLE) {
            vkDestroyDevice(vk, nullptr);
            vk = VK_NULL_HANDLE;
        }
    }

    bool VulkanDevice::isValid() const {
        return vk != nullptr;
    }

    // VulkanInterface

    VulkanInterface::VulkanInterface() {
        VkResult res = volkInitialize();
        if (res != VK_SUCCESS) {
            fprintf(stderr, "volkInitialize failed with error code 0x%X.\n", res);
            return;
        }

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "RT64";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "RT64";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledLayerCount = 0;

        // Check for extensions.
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

        std::unordered_set<std::string> missingRequiredExtensions = RequiredInstanceExtensions;
        std::unordered_set<std::string> supportedOptionalExtensions;
#   if DLSS_ENABLED
        const std::unordered_set<std::string> dlssExtensions = DLSS::getRequiredInstanceExtensionsVulkan();
#   endif
        for (uint32_t i = 0; i < extensionCount; i++) {
            const std::string extensionName(availableExtensions[i].extensionName);
            missingRequiredExtensions.erase(extensionName);

            if (OptionalInstanceExtensions.find(extensionName) != OptionalInstanceExtensions.end()) {
                supportedOptionalExtensions.insert(extensionName);
            }
#       if DLSS_ENABLED
            else if (dlssExtensions.find(extensionName) != dlssExtensions.end()) {
                supportedOptionalExtensions.insert(extensionName);
            }
#       endif
        }

        if (!missingRequiredExtensions.empty()) {
            for (const std::string &extension : missingRequiredExtensions) {
                fprintf(stderr, "Missing required extension: %s.\n", extension.c_str());
            }

            fprintf(stderr, "Unable to create instance. Required extensions are missing.\n");
            return;
        }

        std::vector<const char *> enabledExtensions;
        for (const std::string &extension : RequiredInstanceExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        for (const std::string &extension : supportedOptionalExtensions) {
            enabledExtensions.push_back(extension.c_str());
        }

        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
        createInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());

#   ifdef VULKAN_VALIDATION_LAYER_ENABLED
        // Search for validation layer and enabled it.
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        
        const char validationLayerName[] = "VK_LAYER_KHRONOS_validation";
        const char *enabledLayerNames[] = { validationLayerName };
        for (const VkLayerProperties &layerProperties : availableLayers) {
            if (strcmp(layerProperties.layerName, validationLayerName) == 0) {
                createInfo.ppEnabledLayerNames = enabledLayerNames;
                createInfo.enabledLayerCount = 1;
                break;
            }
        }
#   endif
        
        res = vkCreateInstance(&createInfo, nullptr, &instance);
        if (res != VK_SUCCESS) {
            fprintf(stderr, "vkCreateInstance failed with error code 0x%X.\n", res);
            return;
        }

        volkLoadInstance(instance);

        // Fill capabilities.
        capabilities.shaderFormat = RenderShaderFormat::SPIRV;
    }

    VulkanInterface::~VulkanInterface() {
        if (instance != nullptr) {
            vkDestroyInstance(instance, nullptr);
        }
    }

    std::unique_ptr<RenderDevice> VulkanInterface::createDevice() {
        std::unique_ptr<VulkanDevice> createdDevice = std::make_unique<VulkanDevice>(this);
        return createdDevice->isValid() ? std::move(createdDevice) : nullptr;
    }

    const RenderInterfaceCapabilities &VulkanInterface::getCapabilities() const {
        return capabilities;
    }

    bool VulkanInterface::isValid() const {
        return instance != nullptr;
    }

    // Global creation function.

    std::unique_ptr<RenderInterface> CreateVulkanInterface() {
        std::unique_ptr<VulkanInterface> createdInterface = std::make_unique<VulkanInterface>();
        return createdInterface->isValid() ? std::move(createdInterface) : nullptr;
    }
};
