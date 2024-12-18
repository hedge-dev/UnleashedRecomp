//
// plume
//
// Copyright (c) 2024 renderbag and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file for details.
//

#include "plume_d3d12.h"

#include <unordered_set>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#pragma clang diagnostic ignored "-Wswitch"
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifndef NDEBUG
#   define D3D12_DEBUG_LAYER_ENABLED
#   define D3D12_DEBUG_LAYER_BREAK_ON_ERROR true
#   define D3D12_DEBUG_LAYER_BREAK_ON_WARNING false
#   define D3D12_DEBUG_LAYER_SUPRESS_SAMPLE_POSITIONS_ERROR // Supress error message that's been fixed in newer Agility SDK versions.
//#   define D3D12_DEBUG_LAYER_GPU_BASED_VALIDATION_ENABLED
#endif

// Old Windows SDK versions don't provide this macro, so we workaround it by making sure it is defined.
#ifndef D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE
#define D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
#endif

extern "C" {
    __declspec(dllexport) extern const UINT D3D12SDKVersion = D3D12_SDK_VERSION;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\";
}

namespace plume {
    static const uint32_t ShaderDescriptorHeapSize = 65536;
    static const uint32_t SamplerDescriptorHeapSize = 1024;
    static const uint32_t TargetDescriptorHeapSize = 16384;

    // Common functions.

    static std::wstring Utf8ToUtf16(const std::string_view& value) {
        std::wstring wideStr;
        wideStr.resize(MultiByteToWideChar(CP_UTF8, 0, value.data(), value.size(), nullptr, 0));
        MultiByteToWideChar(CP_UTF8, 0, value.data(), value.size(), wideStr.data(), wideStr.size());
        return wideStr;
    }

    static std::string Utf16ToUtf8(const std::wstring_view& value) {
        std::string multiByteStr;
        multiByteStr.resize(WideCharToMultiByte(CP_UTF8, 0, value.data(), value.size(), nullptr, 0, nullptr, FALSE));
        WideCharToMultiByte(CP_UTF8, 0, value.data(), value.size(), multiByteStr.data(), multiByteStr.size(), nullptr, FALSE);
        return multiByteStr;
    }

    static uint32_t roundUp(uint32_t value, uint32_t powerOf2Alignment) {
        return (value + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
    }

    static uint64_t roundUp(uint64_t value, uint64_t powerOf2Alignment) {
        return (value + powerOf2Alignment - 1) & ~(powerOf2Alignment - 1);
    }

    static DXGI_FORMAT toDXGI(RenderFormat format) {
        switch (format) {
        case RenderFormat::UNKNOWN:
            return DXGI_FORMAT_UNKNOWN;
        case RenderFormat::R32G32B32A32_TYPELESS:
            return DXGI_FORMAT_R32G32B32A32_TYPELESS;
        case RenderFormat::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RenderFormat::R32G32B32A32_UINT:
            return DXGI_FORMAT_R32G32B32A32_UINT;
        case RenderFormat::R32G32B32A32_SINT:
            return DXGI_FORMAT_R32G32B32A32_SINT;
        case RenderFormat::R32G32B32_TYPELESS:
            return DXGI_FORMAT_R32G32B32_TYPELESS;
        case RenderFormat::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case RenderFormat::R32G32B32_UINT:
            return DXGI_FORMAT_R32G32B32_UINT;
        case RenderFormat::R32G32B32_SINT:
            return DXGI_FORMAT_R32G32B32_SINT;
        case RenderFormat::R16G16B16A16_TYPELESS:
            return DXGI_FORMAT_R16G16B16A16_TYPELESS;
        case RenderFormat::R16G16B16A16_FLOAT:
            return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case RenderFormat::R16G16B16A16_UNORM:
            return DXGI_FORMAT_R16G16B16A16_UNORM;
        case RenderFormat::R16G16B16A16_UINT:
            return DXGI_FORMAT_R16G16B16A16_UINT;
        case RenderFormat::R16G16B16A16_SNORM:
            return DXGI_FORMAT_R16G16B16A16_SNORM;
        case RenderFormat::R16G16B16A16_SINT:
            return DXGI_FORMAT_R16G16B16A16_SINT;
        case RenderFormat::R32G32_TYPELESS:
            return DXGI_FORMAT_R32G32_TYPELESS;
        case RenderFormat::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case RenderFormat::R32G32_UINT:
            return DXGI_FORMAT_R32G32_UINT;
        case RenderFormat::R32G32_SINT:
            return DXGI_FORMAT_R32G32_SINT;
        case RenderFormat::R8G8B8A8_TYPELESS:
            return DXGI_FORMAT_R8G8B8A8_TYPELESS;
        case RenderFormat::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RenderFormat::R8G8B8A8_UINT:
            return DXGI_FORMAT_R8G8B8A8_UINT;
        case RenderFormat::R8G8B8A8_SNORM:
            return DXGI_FORMAT_R8G8B8A8_SNORM;
        case RenderFormat::R8G8B8A8_SINT:
            return DXGI_FORMAT_R8G8B8A8_SINT;
        case RenderFormat::B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        case RenderFormat::R16G16_TYPELESS:
            return DXGI_FORMAT_R16G16_TYPELESS;
        case RenderFormat::R16G16_FLOAT:
            return DXGI_FORMAT_R16G16_FLOAT;
        case RenderFormat::R16G16_UNORM:
            return DXGI_FORMAT_R16G16_UNORM;
        case RenderFormat::R16G16_UINT:
            return DXGI_FORMAT_R16G16_UINT;
        case RenderFormat::R16G16_SNORM:
            return DXGI_FORMAT_R16G16_SNORM;
        case RenderFormat::R16G16_SINT:
            return DXGI_FORMAT_R16G16_SINT;
        case RenderFormat::R32_TYPELESS:
            return DXGI_FORMAT_R32_TYPELESS;
        case RenderFormat::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        case RenderFormat::R32_FLOAT:
            return DXGI_FORMAT_R32_FLOAT;
        case RenderFormat::R32_UINT:
            return DXGI_FORMAT_R32_UINT;
        case RenderFormat::R32_SINT:
            return DXGI_FORMAT_R32_SINT;
        case RenderFormat::R8G8_TYPELESS:
            return DXGI_FORMAT_R8G8_TYPELESS;
        case RenderFormat::R8G8_UNORM:
            return DXGI_FORMAT_R8G8_UNORM;
        case RenderFormat::R8G8_UINT:
            return DXGI_FORMAT_R8G8_UINT;
        case RenderFormat::R8G8_SNORM:
            return DXGI_FORMAT_R8G8_SNORM;
        case RenderFormat::R8G8_SINT:
            return DXGI_FORMAT_R8G8_SINT;
        case RenderFormat::R16_TYPELESS:
            return DXGI_FORMAT_R16_TYPELESS;
        case RenderFormat::R16_FLOAT:
            return DXGI_FORMAT_R16_FLOAT;
        case RenderFormat::D16_UNORM:
            return DXGI_FORMAT_D16_UNORM;
        case RenderFormat::R16_UNORM:
            return DXGI_FORMAT_R16_UNORM;
        case RenderFormat::R16_UINT:
            return DXGI_FORMAT_R16_UINT;
        case RenderFormat::R16_SNORM:
            return DXGI_FORMAT_R16_SNORM;
        case RenderFormat::R16_SINT:
            return DXGI_FORMAT_R16_SINT;
        case RenderFormat::R8_TYPELESS:
            return DXGI_FORMAT_R8_TYPELESS;
        case RenderFormat::R8_UNORM:
            return DXGI_FORMAT_R8_UNORM;
        case RenderFormat::R8_UINT:
            return DXGI_FORMAT_R8_UINT;
        case RenderFormat::R8_SNORM:
            return DXGI_FORMAT_R8_SNORM;
        case RenderFormat::R8_SINT:
            return DXGI_FORMAT_R8_SINT;
        case RenderFormat::BC1_TYPELESS:
            return DXGI_FORMAT_BC1_TYPELESS;
        case RenderFormat::BC1_UNORM:
            return DXGI_FORMAT_BC1_UNORM;
        case RenderFormat::BC1_UNORM_SRGB:
            return DXGI_FORMAT_BC1_UNORM_SRGB;
        case RenderFormat::BC2_TYPELESS:
            return DXGI_FORMAT_BC2_TYPELESS;
        case RenderFormat::BC2_UNORM:
            return DXGI_FORMAT_BC2_UNORM;
        case RenderFormat::BC2_UNORM_SRGB:
            return DXGI_FORMAT_BC2_UNORM_SRGB;
        case RenderFormat::BC3_TYPELESS:
            return DXGI_FORMAT_BC3_TYPELESS;
        case RenderFormat::BC3_UNORM:
            return DXGI_FORMAT_BC3_UNORM;
        case RenderFormat::BC3_UNORM_SRGB:
            return DXGI_FORMAT_BC3_UNORM_SRGB;
        case RenderFormat::BC4_TYPELESS:
            return DXGI_FORMAT_BC4_TYPELESS;
        case RenderFormat::BC4_UNORM:
            return DXGI_FORMAT_BC4_UNORM;
        case RenderFormat::BC4_SNORM:
            return DXGI_FORMAT_BC4_SNORM;
        case RenderFormat::BC5_TYPELESS:
            return DXGI_FORMAT_BC5_TYPELESS;
        case RenderFormat::BC5_UNORM:
            return DXGI_FORMAT_BC5_UNORM;
        case RenderFormat::BC5_SNORM:
            return DXGI_FORMAT_BC5_SNORM;
        case RenderFormat::BC6H_TYPELESS:
            return DXGI_FORMAT_BC6H_TYPELESS;
        case RenderFormat::BC6H_UF16:
            return DXGI_FORMAT_BC6H_UF16;
        case RenderFormat::BC6H_SF16:
            return DXGI_FORMAT_BC6H_SF16;
        case RenderFormat::BC7_TYPELESS:
            return DXGI_FORMAT_BC7_TYPELESS;
        case RenderFormat::BC7_UNORM:
            return DXGI_FORMAT_BC7_UNORM;
        case RenderFormat::BC7_UNORM_SRGB:
            return DXGI_FORMAT_BC7_UNORM_SRGB;
        default:
            assert(false && "Unknown format.");
            return DXGI_FORMAT_FORCE_UINT;
        }
    }

    static D3D12_BLEND toD3D12(RenderBlend blend) {
        switch (blend) {
        case RenderBlend::ZERO:
            return D3D12_BLEND_ZERO;
        case RenderBlend::ONE:
            return D3D12_BLEND_ONE;
        case RenderBlend::SRC_COLOR:
            return D3D12_BLEND_SRC_COLOR;
        case RenderBlend::INV_SRC_COLOR:
            return D3D12_BLEND_INV_SRC_COLOR;
        case RenderBlend::SRC_ALPHA:
            return D3D12_BLEND_SRC_ALPHA;
        case RenderBlend::INV_SRC_ALPHA:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case RenderBlend::DEST_ALPHA:
            return D3D12_BLEND_DEST_ALPHA;
        case RenderBlend::INV_DEST_ALPHA:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case RenderBlend::DEST_COLOR:
            return D3D12_BLEND_DEST_COLOR;
        case RenderBlend::INV_DEST_COLOR:
            return D3D12_BLEND_INV_DEST_COLOR;
        case RenderBlend::SRC_ALPHA_SAT:
            return D3D12_BLEND_SRC_ALPHA_SAT;
        case RenderBlend::BLEND_FACTOR:
            return D3D12_BLEND_BLEND_FACTOR;
        case RenderBlend::INV_BLEND_FACTOR:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case RenderBlend::SRC1_COLOR:
            return D3D12_BLEND_SRC1_COLOR;
        case RenderBlend::INV_SRC1_COLOR:
            return D3D12_BLEND_INV_SRC1_COLOR;
        case RenderBlend::SRC1_ALPHA:
            return D3D12_BLEND_SRC1_ALPHA;
        case RenderBlend::INV_SRC1_ALPHA:
            return D3D12_BLEND_INV_SRC1_ALPHA;
        default:
            assert(false && "Unknown blend.");
            return D3D12_BLEND_ZERO;
        }
    }

    static D3D12_BLEND_OP toD3D12(RenderBlendOperation operation) {
        switch (operation) {
        case RenderBlendOperation::ADD:
            return D3D12_BLEND_OP_ADD;
        case RenderBlendOperation::SUBTRACT:
            return D3D12_BLEND_OP_SUBTRACT;
        case RenderBlendOperation::REV_SUBTRACT:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case RenderBlendOperation::MIN:
            return D3D12_BLEND_OP_MIN;
        case RenderBlendOperation::MAX:
            return D3D12_BLEND_OP_MAX;
        default:
            assert(false && "Unknown blend operation.");
            return D3D12_BLEND_OP_ADD;
        }
    }
    
    static D3D12_COLOR_WRITE_ENABLE toD3D12(RenderColorWriteEnable enable) {
        return D3D12_COLOR_WRITE_ENABLE(
            ((uint32_t(enable) & uint32_t(RenderColorWriteEnable::RED)) ? D3D12_COLOR_WRITE_ENABLE_RED : 0x0) |
            ((uint32_t(enable) & uint32_t(RenderColorWriteEnable::GREEN)) ? D3D12_COLOR_WRITE_ENABLE_GREEN : 0x0) |
            ((uint32_t(enable) & uint32_t(RenderColorWriteEnable::BLUE)) ? D3D12_COLOR_WRITE_ENABLE_BLUE : 0x0) |
            ((uint32_t(enable) & uint32_t(RenderColorWriteEnable::ALPHA)) ? D3D12_COLOR_WRITE_ENABLE_ALPHA : 0x0)
        );
    }

    static D3D12_LOGIC_OP toD3D12(RenderLogicOperation operation) {
        switch (operation) {
        case RenderLogicOperation::CLEAR:
            return D3D12_LOGIC_OP_CLEAR;
        case RenderLogicOperation::SET:
            return D3D12_LOGIC_OP_SET;
        case RenderLogicOperation::COPY:
            return D3D12_LOGIC_OP_COPY;
        case RenderLogicOperation::COPY_INVERTED:
            return D3D12_LOGIC_OP_COPY_INVERTED;
        case RenderLogicOperation::NOOP:
            return D3D12_LOGIC_OP_NOOP;
        case RenderLogicOperation::INVERT:
            return D3D12_LOGIC_OP_INVERT;
        case RenderLogicOperation::AND:
            return D3D12_LOGIC_OP_AND;
        case RenderLogicOperation::NAND:
            return D3D12_LOGIC_OP_NAND;
        case RenderLogicOperation::OR:
            return D3D12_LOGIC_OP_OR;
        case RenderLogicOperation::NOR:
            return D3D12_LOGIC_OP_NOR;
        case RenderLogicOperation::XOR:
            return D3D12_LOGIC_OP_XOR;
        case RenderLogicOperation::EQUIV:
            return D3D12_LOGIC_OP_EQUIV;
        case RenderLogicOperation::AND_REVERSE:
            return D3D12_LOGIC_OP_AND_REVERSE;
        case RenderLogicOperation::AND_INVERTED:
            return D3D12_LOGIC_OP_AND_INVERTED;
        case RenderLogicOperation::OR_REVERSE:
            return D3D12_LOGIC_OP_OR_REVERSE;
        case RenderLogicOperation::OR_INVERTED:
            return D3D12_LOGIC_OP_OR_INVERTED;
        default:
            assert(false && "Unknown logic operation.");
            return D3D12_LOGIC_OP_CLEAR;
        }
    }
    
    static D3D12_FILTER toFilter(RenderFilter minFilter, RenderFilter magFilter, RenderMipmapMode mipmapMode, bool anisotropyEnabled, bool comparisonEnabled) {
        assert(minFilter != RenderFilter::UNKNOWN);
        assert(magFilter != RenderFilter::UNKNOWN);
        assert(mipmapMode != RenderMipmapMode::UNKNOWN);

        if (anisotropyEnabled) {
            return comparisonEnabled ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
        }
        else {
            uint32_t filterInt = 0;
            filterInt |= (mipmapMode == RenderMipmapMode::LINEAR) ? 0x1 : 0x0;
            filterInt |= (magFilter == RenderFilter::LINEAR) ? 0x4 : 0x0;
            filterInt |= (minFilter == RenderFilter::LINEAR) ? 0x10 : 0x0;
            filterInt |= comparisonEnabled ? 0x80 : 0x0;
            return D3D12_FILTER(filterInt);
        }
    }

    static D3D12_TEXTURE_ADDRESS_MODE toD3D12(RenderTextureAddressMode addressMode) {
        switch (addressMode) {
        case RenderTextureAddressMode::WRAP:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case RenderTextureAddressMode::MIRROR:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case RenderTextureAddressMode::CLAMP:
            return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case RenderTextureAddressMode::BORDER:
            return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case RenderTextureAddressMode::MIRROR_ONCE:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
        default:
            assert(false && "Unknown texture address mode.");
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }
    }

    static D3D12_STATIC_BORDER_COLOR toStaticBorderColor(RenderBorderColor borderColor) {
        switch (borderColor) {
        case RenderBorderColor::TRANSPARENT_BLACK:
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        case RenderBorderColor::OPAQUE_BLACK:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
        case RenderBorderColor::OPAQUE_WHITE:
            return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        default:
            assert(false && "Unknown static border color.");
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        }
    }

    static D3D12_SHADER_VISIBILITY toD3D12(RenderShaderVisibility visibility) {
        switch (visibility) {
        case RenderShaderVisibility::ALL:
            return D3D12_SHADER_VISIBILITY_ALL;
        case RenderShaderVisibility::VERTEX:
            return D3D12_SHADER_VISIBILITY_VERTEX;
        case RenderShaderVisibility::GEOMETRY:
            return D3D12_SHADER_VISIBILITY_GEOMETRY;
        case RenderShaderVisibility::PIXEL:
            return D3D12_SHADER_VISIBILITY_PIXEL;
        default:
            assert(false && "Unknown shader visibility.");
            return D3D12_SHADER_VISIBILITY_ALL;
        }
    }

    static D3D12_INPUT_CLASSIFICATION toD3D12(RenderInputSlotClassification classification) {
        switch (classification) {
        case RenderInputSlotClassification::PER_VERTEX_DATA:
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        case RenderInputSlotClassification::PER_INSTANCE_DATA:
            return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
        default:
            assert(false && "Unknown input classification.");
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        }
    }

    static D3D12_DESCRIPTOR_RANGE_TYPE toRangeType(RenderDescriptorRangeType type) {
        switch (type) {
        case RenderDescriptorRangeType::FORMATTED_BUFFER:
        case RenderDescriptorRangeType::TEXTURE:
        case RenderDescriptorRangeType::STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER:
        case RenderDescriptorRangeType::ACCELERATION_STRUCTURE:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        case RenderDescriptorRangeType::READ_WRITE_FORMATTED_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_TEXTURE:
        case RenderDescriptorRangeType::READ_WRITE_STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
        case RenderDescriptorRangeType::CONSTANT_BUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case RenderDescriptorRangeType::SAMPLER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
        default:
            assert(false && "Unknown descriptor range type.");
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }
    }

    static D3D12_HEAP_TYPE toD3D12(RenderHeapType type) {
        switch (type) {
        case RenderHeapType::DEFAULT:
            return D3D12_HEAP_TYPE_DEFAULT;
        case RenderHeapType::UPLOAD:
            return D3D12_HEAP_TYPE_UPLOAD;
        case RenderHeapType::READBACK:
            return D3D12_HEAP_TYPE_READBACK;
        default:
            assert(false && "Unknown heap type.");
            return D3D12_HEAP_TYPE_DEFAULT;
        }
    }

    static D3D12_COMPARISON_FUNC toD3D12(RenderComparisonFunction function) {
        switch (function) {
        case RenderComparisonFunction::NEVER:
            return D3D12_COMPARISON_FUNC_NEVER;
        case RenderComparisonFunction::LESS:
            return D3D12_COMPARISON_FUNC_LESS;
        case RenderComparisonFunction::EQUAL:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case RenderComparisonFunction::LESS_EQUAL:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case RenderComparisonFunction::GREATER:
            return D3D12_COMPARISON_FUNC_GREATER;
        case RenderComparisonFunction::NOT_EQUAL:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case RenderComparisonFunction::GREATER_EQUAL:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case RenderComparisonFunction::ALWAYS:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            assert(false && "Unknown comparison function.");
            return D3D12_COMPARISON_FUNC_NEVER;
        }
    }

    static D3D12_PRIMITIVE_TOPOLOGY toD3D12(RenderPrimitiveTopology topology) {
        switch (topology) {
        case RenderPrimitiveTopology::POINT_LIST:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case RenderPrimitiveTopology::LINE_LIST:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;   
        case RenderPrimitiveTopology::LINE_STRIP:
            return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case RenderPrimitiveTopology::TRIANGLE_LIST:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;     
        case RenderPrimitiveTopology::TRIANGLE_STRIP:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;     
        case RenderPrimitiveTopology::TRIANGLE_FAN:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
        default:
            assert(false && "Unknown primitive topology.");
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE toTopologyType(RenderPrimitiveTopology topologyType) {
        switch (topologyType) {
        case RenderPrimitiveTopology::POINT_LIST:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case RenderPrimitiveTopology::LINE_LIST:
        case RenderPrimitiveTopology::LINE_STRIP:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case RenderPrimitiveTopology::TRIANGLE_LIST:
        case RenderPrimitiveTopology::TRIANGLE_STRIP:
        case RenderPrimitiveTopology::TRIANGLE_FAN:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            assert(false && "Unknown primitive topology type.");
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }
    }

    static D3D12_RESOURCE_DIMENSION toD3D12(RenderTextureDimension dimension) {
        switch (dimension) {
        case RenderTextureDimension::UNKNOWN:
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        case RenderTextureDimension::TEXTURE_1D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case RenderTextureDimension::TEXTURE_2D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case RenderTextureDimension::TEXTURE_3D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            assert(false && "Unknown resource dimension.");
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }
    }

    static D3D12_TEXTURE_LAYOUT toD3D12(RenderTextureArrangement arrangement) {
        switch (arrangement) {
        case RenderTextureArrangement::UNKNOWN:
            return D3D12_TEXTURE_LAYOUT_UNKNOWN;
        case RenderTextureArrangement::ROW_MAJOR:
            return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        default:
            assert(false && "Unknown texture arrangement.");
            return D3D12_TEXTURE_LAYOUT_UNKNOWN;
        }
    }

    static D3D12_RESOURCE_STATES toBufferState(RenderBarrierStages stages, RenderBufferAccessBits accessBits, RenderBufferFlags bufferFlags) {
        // The only allowed state for acceleration structures.
        if (bufferFlags & RenderBufferFlag::ACCELERATION_STRUCTURE) {
            return D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }

        // Use copy-optimized states.
        if (stages == RenderBarrierStage::COPY) {
            if (accessBits == RenderBufferAccess::WRITE) {
                return D3D12_RESOURCE_STATE_COPY_DEST;
            }
            else if (accessBits == RenderBufferAccess::READ) {
                return D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
        }

        // Use unordered access state if the buffer supports it and writing is enabled.
        if ((accessBits & RenderBufferAccess::WRITE) && (bufferFlags & RenderBufferFlag::UNORDERED_ACCESS)) {
            return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }

        // If both stages are required and the buffer is read-only, use the all shader resource state.
        if (stages == (RenderBarrierStage::GRAPHICS | RenderBarrierStage::COMPUTE)) {
            if (accessBits == RenderBufferAccess::READ) {
                return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
            }
        }

        // Use graphics pipeline states.
        if (stages == RenderBarrierStage::GRAPHICS) {
            if (accessBits == RenderBufferAccess::READ) {
                if (bufferFlags & (RenderBufferFlag::VERTEX | RenderBufferFlag::CONSTANT)) {
                    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
                }

                if (bufferFlags & RenderBufferFlag::INDEX) {
                    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
                }

                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            }
        }

        // Fall back to common state.
        return D3D12_RESOURCE_STATE_COMMON;
    }
    
    static D3D12_RESOURCE_STATES toTextureState(RenderBarrierStages stages, RenderTextureLayout textureLayout, RenderTextureFlags textureFlags) {
        switch (textureLayout) {
        case RenderTextureLayout::GENERAL:
            return (textureFlags & RenderTextureFlag::UNORDERED_ACCESS) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON;
        case RenderTextureLayout::SHADER_READ:
            switch (stages) {
            case RenderBarrierStage::GRAPHICS:
                return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            case RenderBarrierStage::COMPUTE:
                return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            default:
                return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
            }
        case RenderTextureLayout::COLOR_WRITE:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case RenderTextureLayout::DEPTH_WRITE:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case RenderTextureLayout::DEPTH_READ:
            return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ;
        case RenderTextureLayout::COPY_SOURCE:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case RenderTextureLayout::COPY_DEST:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        case RenderTextureLayout::RESOLVE_SOURCE:
            return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
        case RenderTextureLayout::RESOLVE_DEST:
            return D3D12_RESOURCE_STATE_RESOLVE_DEST;
        case RenderTextureLayout::PRESENT:
            return D3D12_RESOURCE_STATE_PRESENT;
        default:
            assert(false && "Unknown texture layout.");
            return D3D12_RESOURCE_STATE_COMMON;
        }
    }

    static D3D12_TEXTURE_COPY_LOCATION toD3D12(const RenderTextureCopyLocation &location) {
        D3D12_TEXTURE_COPY_LOCATION loc;
        switch (location.type) {
        case RenderTextureCopyType::SUBRESOURCE: {
            const D3D12Texture *interfaceTexture = static_cast<const D3D12Texture *>(location.texture);
            loc.pResource = (interfaceTexture != nullptr) ? interfaceTexture->d3d : nullptr;
            loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            loc.SubresourceIndex = location.subresource.index;
            break;
        }
        case RenderTextureCopyType::PLACED_FOOTPRINT: {
            const D3D12Buffer *interfaceBuffer = static_cast<const D3D12Buffer *>(location.buffer);
            const uint32_t blockWidth = RenderFormatBlockWidth(location.placedFootprint.format);
            const uint32_t blockCount = (location.placedFootprint.rowWidth + blockWidth - 1) / blockWidth;
            loc.pResource = (interfaceBuffer != nullptr) ? interfaceBuffer->d3d : nullptr;
            loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            loc.PlacedFootprint.Offset = location.placedFootprint.offset;
            loc.PlacedFootprint.Footprint.Format = toDXGI(location.placedFootprint.format);
            loc.PlacedFootprint.Footprint.Width = ((location.placedFootprint.width + blockWidth - 1) / blockWidth) * blockWidth;
            loc.PlacedFootprint.Footprint.Height = ((location.placedFootprint.height + blockWidth - 1) / blockWidth) * blockWidth;
            loc.PlacedFootprint.Footprint.Depth = location.placedFootprint.depth;
            loc.PlacedFootprint.Footprint.RowPitch = blockCount * RenderFormatSize(location.placedFootprint.format);

            // Test for conditions that might not be reported if the hardware doesn't complain about them.
            assert(((loc.PlacedFootprint.Offset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) == 0) && "Resulting offset must be aligned to 512 bytes in D3D12.");
            assert(((loc.PlacedFootprint.Footprint.RowPitch % D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) == 0) && "Resulting row pitch must be aligned to 256 bytes in D3D12.");

            break;
        }
        default: {
            assert(false && "Unknown texture copy type.");
        }
        }

        return loc;
    }

    static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS toRTASBuildFlags(bool preferFastBuild, bool preferFastTrace) {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        flags |= preferFastBuild ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        flags |= preferFastTrace ? D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE : D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        return flags;
    }

    static UINT toD3D12(RenderSwizzle swizzle, UINT identity) {
        switch (swizzle) {
        case RenderSwizzle::IDENTITY:
            return identity;
        case RenderSwizzle::ZERO:
            return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;  
        case RenderSwizzle::ONE:
            return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;    
        case RenderSwizzle::R:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;   
        case RenderSwizzle::G:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;     
        case RenderSwizzle::B:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;    
        case RenderSwizzle::A:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;
        default:
            assert(false && "Unknown swizzle type.");
            return identity;
        }
    }

    static UINT toD3D12(const RenderComponentMapping &componentMapping) {
        return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
            toD3D12(componentMapping.r, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0),
            toD3D12(componentMapping.g, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1),
            toD3D12(componentMapping.b, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2),
            toD3D12(componentMapping.a, D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3)
        );
    }

    static void setObjectName(ID3D12Object *object, const std::string &name) {
        const std::wstring wideCharName = Utf8ToUtf16(name);
        object->SetName(wideCharName.c_str());
    }

    // D3D12DescriptorHeapAllocator

    D3D12DescriptorHeapAllocator::D3D12DescriptorHeapAllocator(D3D12Device *device, uint32_t heapSize, D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
        assert(device != nullptr);
        assert(heapSize > 0);

        this->device = device;
        this->heapSize = heapSize;
        this->freeSize = heapSize;

        D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
        heapDesc.NumDescriptors = heapSize;
        heapDesc.Type = heapType;
        descriptorHandleIncrement = device->d3d->GetDescriptorHandleIncrementSize(heapDesc.Type);

        const bool shaderVisible = (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) || (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        if (shaderVisible) {
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        }

        HRESULT res = device->d3d->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));
        if (FAILED(res)) {
            fprintf(stderr, "CreateDescriptorHeap failed with error code 0x%lX.\n", res);
            return;
        }
        
        cpuDescriptorHandle = heap->GetCPUDescriptorHandleForHeapStart();

        if (shaderVisible) {
            gpuDescriptorHandle = heap->GetGPUDescriptorHandleForHeapStart();
        }

        addFreeBlock(0, heapSize);
    }

    D3D12DescriptorHeapAllocator::~D3D12DescriptorHeapAllocator() {
        if (heap != nullptr) {
            heap->Release();
        }
    }

    void D3D12DescriptorHeapAllocator::addFreeBlock(uint32_t offset, uint32_t size) {
        OffsetFreeBlockMap::iterator blockOffsetIt = offsetFreeBlockMap.emplace(offset, size).first;
        SizeFreeBlockMap::iterator blockSizeIt = sizeFreeBlockMap.emplace(size, blockOffsetIt);
        blockOffsetIt->second.sizeMapIterator = blockSizeIt;
    }

    uint32_t D3D12DescriptorHeapAllocator::allocate(uint32_t size) {
        const std::scoped_lock lock(allocationMutex);
        if (freeSize < size) {
            return INVALID_OFFSET;
        }

        SizeFreeBlockMap::iterator blockSizeIt = sizeFreeBlockMap.lower_bound(size);
        if (blockSizeIt == sizeFreeBlockMap.end()) {
            return INVALID_OFFSET;
        }

        OffsetFreeBlockMap::iterator blockOffsetIt = blockSizeIt->second;
        uint32_t retOffset = blockOffsetIt->first;
        uint32_t newOffset = retOffset + size;
        uint32_t newSize = blockOffsetIt->second.size - size;
        sizeFreeBlockMap.erase(blockSizeIt);
        offsetFreeBlockMap.erase(blockOffsetIt);
        if (newSize > 0) {
            addFreeBlock(newOffset, newSize);
        }

        freeSize -= size;
        return retOffset;
    }

    void D3D12DescriptorHeapAllocator::free(uint32_t offset, uint32_t size) {
        const std::scoped_lock lock(allocationMutex);
        OffsetFreeBlockMap::iterator nextBlockIt = offsetFreeBlockMap.upper_bound(offset);
        OffsetFreeBlockMap::iterator prevBlockIt = nextBlockIt;
        if (prevBlockIt != offsetFreeBlockMap.begin()) {
            prevBlockIt--;
        }
        else {
            prevBlockIt = offsetFreeBlockMap.end();
        }

        freeSize += size;

        // The previous free block is contiguous.
        if ((prevBlockIt != offsetFreeBlockMap.end()) && (offset == (prevBlockIt->first + prevBlockIt->second.size))) {
            size = prevBlockIt->second.size + size;
            offset = prevBlockIt->first;
            sizeFreeBlockMap.erase(prevBlockIt->second.sizeMapIterator);
            offsetFreeBlockMap.erase(prevBlockIt);
        }

        // The next free block is contiguous.
        if ((nextBlockIt != offsetFreeBlockMap.end()) && ((offset + size) == nextBlockIt->first)) {
            size = size + nextBlockIt->second.size;
            sizeFreeBlockMap.erase(nextBlockIt->second.sizeMapIterator);
            offsetFreeBlockMap.erase(nextBlockIt);
        }

        addFreeBlock(offset, size);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocator::getCPUHandleAt(uint32_t index) const {
        assert(index < heapSize);
        assert(cpuDescriptorHandle.ptr > 0);
        return { cpuDescriptorHandle.ptr + uint64_t(index) * descriptorHandleIncrement };
    }

    D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorHeapAllocator::getGPUHandleAt(uint32_t index) const {
        assert(index < heapSize);
        assert(gpuDescriptorHandle.ptr > 0);
        return { gpuDescriptorHandle.ptr + uint64_t(index) * descriptorHandleIncrement };
    }

    // D3D12DescriptorSet

    D3D12DescriptorSet::D3D12DescriptorSet(D3D12Device *device, const RenderDescriptorSetDesc &desc) {
        assert(device != nullptr);

        this->device = device;

        // Figure out the total amount of entries that will be required.
        uint32_t rangeCount = desc.descriptorRangesCount;
        uint32_t viewDescriptorCount = 0;
        uint32_t samplerDescriptorCount = 0;
        auto addDescriptor = [&](const RenderDescriptorRange &range, uint32_t descriptorCount) {
            descriptorTypes.emplace_back(range.type);

            bool isDynamicSampler = (range.type == RenderDescriptorRangeType::SAMPLER) && (range.immutableSampler == nullptr);
            if (isDynamicSampler) {
                descriptorHeapIndices.emplace_back(samplerDescriptorCount);
                samplerDescriptorCount += descriptorCount;
            }
            else {
                descriptorHeapIndices.emplace_back(viewDescriptorCount);
                viewDescriptorCount += descriptorCount;
            }
        };

        if (desc.lastRangeIsBoundless) {
            assert((desc.descriptorRangesCount > 0) && "There must be at least one descriptor set to define the last range as boundless.");
            rangeCount--;
        }

        for (uint32_t i = 0; i < rangeCount; i++) {
            const RenderDescriptorRange &range = desc.descriptorRanges[i];
            for (uint32_t j = 0; j < range.count; j++) {
                addDescriptor(range, 1);
            }
        }

        if (desc.lastRangeIsBoundless) {
            const RenderDescriptorRange &lastDescriptorRange = desc.descriptorRanges[desc.descriptorRangesCount - 1];
            addDescriptor(lastDescriptorRange, desc.boundlessRangeSize);
        }

        if (!descriptorTypes.empty()) {
            descriptorTypeMaxIndex = uint32_t(descriptorTypes.size()) - 1;
        }

        if (viewDescriptorCount > 0) {
            viewAllocation.offset = device->viewHeapAllocator->allocate(viewDescriptorCount);
            if (viewAllocation.offset == D3D12DescriptorHeapAllocator::INVALID_OFFSET) {
                fprintf(stderr, "Allocator was unable to find free space for the set.");
                return;
            }

            viewAllocation.count = viewDescriptorCount;
        }

        if (samplerDescriptorCount > 0) {
            samplerAllocation.offset = device->samplerHeapAllocator->allocate(samplerDescriptorCount);
            if (samplerAllocation.offset == D3D12DescriptorHeapAllocator::INVALID_OFFSET) {
                fprintf(stderr, "Allocator was unable to find free space for the set.");
                return;
            }

            samplerAllocation.count = samplerDescriptorCount;
        }
    }

    D3D12DescriptorSet::~D3D12DescriptorSet() {
        if (viewAllocation.count > 0) {
            device->viewHeapAllocator->free(viewAllocation.offset, viewAllocation.count);
        }

        if (samplerAllocation.count > 0) {
            device->samplerHeapAllocator->free(samplerAllocation.offset, samplerAllocation.count);
        }
    }

    void D3D12DescriptorSet::setBuffer(uint32_t descriptorIndex, const RenderBuffer *buffer, uint64_t bufferSize, const RenderBufferStructuredView *bufferStructuredView, const RenderBufferFormattedView *bufferFormattedView) {
        const D3D12Buffer *interfaceBuffer = static_cast<const D3D12Buffer *>(buffer);
        ID3D12Resource *nativeResource = (interfaceBuffer != nullptr) ? interfaceBuffer->d3d : nullptr;
        uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
        RenderDescriptorRangeType descriptorType = descriptorTypes[descriptorIndexClamped];
        switch (descriptorType) {
        case RenderDescriptorRangeType::CONSTANT_BUFFER: {
            uint64_t bufferViewSize = bufferSize;
            if ((bufferSize == 0) && (interfaceBuffer != nullptr)) {
                bufferViewSize = interfaceBuffer->desc.size;
            }

            setCBV(descriptorIndex, nativeResource, bufferViewSize);
            break;
        }
        case RenderDescriptorRangeType::FORMATTED_BUFFER: {
            assert((bufferStructuredView == nullptr) && "Can't use structured view on texture buffers.");
            if (nativeResource != nullptr) {
                assert((bufferFormattedView != nullptr) && "A view must be provided for formatted buffers.");
                const D3D12BufferFormattedView *interfaceBufferFormattedView = static_cast<const D3D12BufferFormattedView *>(bufferFormattedView);
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
                srvDesc.Format = toDXGI(interfaceBufferFormattedView->format);
                srvDesc.Buffer.Flags = (descriptorType == RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER) ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;

                // Figure out the number of elements from the format.
                const uint64_t bufferViewSize = (bufferSize > 0) ? bufferSize : interfaceBuffer->desc.size;
                srvDesc.Buffer.NumElements = UINT(bufferViewSize / RenderFormatSize(interfaceBufferFormattedView->format));
                setSRV(descriptorIndex, nativeResource, &srvDesc);
            }
            else {
                setSRV(descriptorIndex, nullptr, nullptr);
            }

            break;
        }
        case RenderDescriptorRangeType::STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER: {
            assert((bufferFormattedView == nullptr) && "Can't use formatted view on byte or structured buffers.");
            if (nativeResource != nullptr) {
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

                const uint64_t bufferViewSize = (bufferSize > 0) ? bufferSize : interfaceBuffer->desc.size;
                if (descriptorType == RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER) {
                    srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                    srvDesc.Buffer.NumElements = UINT(bufferViewSize / 4);
                    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
                }
                else {
                    assert((bufferStructuredView != nullptr) && "A view must be provided for structured buffers.");
                    assert(bufferStructuredView->structureByteStride > 0);
                    srvDesc.Buffer.FirstElement = bufferStructuredView->firstElement;
                    srvDesc.Buffer.StructureByteStride = bufferStructuredView->structureByteStride;
                    srvDesc.Buffer.NumElements = UINT(bufferViewSize / bufferStructuredView->structureByteStride);
                }

                setSRV(descriptorIndex, nativeResource, &srvDesc);
            }
            else {
                setSRV(descriptorIndex, nullptr, nullptr);
            }

            break;
        }
        case RenderDescriptorRangeType::READ_WRITE_FORMATTED_BUFFER: {
            assert((bufferStructuredView == nullptr) && "Can't use structured view on texture buffers.");
            if (nativeResource != nullptr) {
                assert((bufferFormattedView != nullptr) && "A view must be provided for formatted buffers.");
                const D3D12BufferFormattedView *interfaceBufferFormatView = static_cast<const D3D12BufferFormattedView *>(bufferFormattedView);
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
                uavDesc.Format = toDXGI(interfaceBufferFormatView->format);
                uavDesc.Buffer.Flags = (descriptorType == RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER) ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;

                // Figure out the number of elements from the format.
                const uint64_t bufferViewSize = (bufferSize > 0) ? bufferSize : interfaceBuffer->desc.size;
                uavDesc.Buffer.NumElements = UINT(bufferViewSize / RenderFormatSize(interfaceBufferFormatView->format));
                setUAV(descriptorIndex, nativeResource, &uavDesc);
            }
            else {
                setUAV(descriptorIndex, nullptr, nullptr);
            }

            break;
        }
        case RenderDescriptorRangeType::READ_WRITE_STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER: {
            assert((bufferFormattedView == nullptr) && "Can't use formatted view on byte or structured buffers.");
            if (nativeResource != nullptr) {
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

                const uint64_t bufferViewSize = (bufferSize > 0) ? bufferSize : interfaceBuffer->desc.size;
                if (descriptorType == RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER) {
                    uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
                    uavDesc.Buffer.NumElements = UINT(bufferViewSize / 4);
                    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
                }
                else {
                    assert((bufferStructuredView != nullptr) && "A view must be provided for structured buffers.");
                    assert(bufferStructuredView->structureByteStride > 0);
                    uavDesc.Buffer.FirstElement = bufferStructuredView->firstElement;
                    uavDesc.Buffer.StructureByteStride = bufferStructuredView->structureByteStride;
                    uavDesc.Buffer.NumElements = UINT(bufferViewSize / bufferStructuredView->structureByteStride);
                }

                setUAV(descriptorIndex, nativeResource, &uavDesc);
            }
            else {
                setUAV(descriptorIndex, nullptr, nullptr);
            }

            break;
        }
        case RenderDescriptorRangeType::TEXTURE:
        case RenderDescriptorRangeType::READ_WRITE_TEXTURE:
        case RenderDescriptorRangeType::SAMPLER:
        case RenderDescriptorRangeType::ACCELERATION_STRUCTURE:
            assert(false && "Incompatible descriptor type.");
            break;
        default:
            assert(false && "Unknown descriptor type.");
            break;
        }
    }

    void D3D12DescriptorSet::setTexture(uint32_t descriptorIndex, const RenderTexture *texture, const RenderTextureLayout textureLayout, const RenderTextureView *textureView) {
        // Texture state is ignored by D3D12 because image layout information is not required.

        const D3D12Texture *interfaceTexture = static_cast<const D3D12Texture *>(texture);
        ID3D12Resource *nativeResource = (interfaceTexture != nullptr) ? interfaceTexture->d3d : nullptr;
        uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
        RenderDescriptorRangeType descriptorType = descriptorTypes[descriptorIndexClamped];
        switch (descriptorType) {
        case RenderDescriptorRangeType::TEXTURE: {
            if ((nativeResource != nullptr) && (textureView != nullptr)) {
                const D3D12TextureView *interfaceTextureView = static_cast<const D3D12TextureView *>(textureView);
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Shader4ComponentMapping = interfaceTextureView->shader4ComponentMapping;
                srvDesc.Format = interfaceTextureView->format;

                const bool isMSAA = (interfaceTextureView->texture->desc.multisampling.sampleCount > RenderSampleCount::COUNT_1);
                switch (interfaceTextureView->dimension) {
                case RenderTextureViewDimension::TEXTURE_1D:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    srvDesc.Texture1D.MipLevels = interfaceTextureView->mipLevels;
                    break;
                case RenderTextureViewDimension::TEXTURE_2D:
                    if (isMSAA) {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    else {
                        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        srvDesc.Texture2D.MipLevels = interfaceTextureView->mipLevels;
                    }

                    break;
                case RenderTextureViewDimension::TEXTURE_3D:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                    srvDesc.Texture3D.MipLevels = interfaceTextureView->mipLevels;
                    break;
                case RenderTextureViewDimension::TEXTURE_CUBE:
                    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    srvDesc.TextureCube.MipLevels = interfaceTextureView->mipLevels;
                    break;
                default:
                    assert(false && "Unknown texture dimension.");
                    break;
                }

                setSRV(descriptorIndex, nativeResource, &srvDesc);
            }
            else if (nativeResource != nullptr) {
                setSRV(descriptorIndex, nativeResource, nullptr);
            }
            else {
                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                setSRV(descriptorIndex, nullptr, &srvDesc);
            }

            break;
        }
        case RenderDescriptorRangeType::READ_WRITE_TEXTURE: {
            if ((nativeResource != nullptr) && (textureView != nullptr)) {
                const D3D12TextureView *interfaceTextureView = static_cast<const D3D12TextureView *>(textureView);
                D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
                uavDesc.Format = interfaceTextureView->format;

                switch (interfaceTextureView->dimension) {
                case RenderTextureViewDimension::TEXTURE_1D:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
                    uavDesc.Texture1D.MipSlice = interfaceTextureView->mipSlice;
                    break;
                case RenderTextureViewDimension::TEXTURE_2D:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                    uavDesc.Texture2D.MipSlice = interfaceTextureView->mipSlice;
                    break;
                case RenderTextureViewDimension::TEXTURE_3D:
                    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
                    uavDesc.Texture3D.MipSlice = interfaceTextureView->mipSlice;
                    break;
                default:
                    assert(false && "Unknown texture dimension.");
                    break;
                }

                setUAV(descriptorIndex, nativeResource, &uavDesc);
            }
            else {
                setUAV(descriptorIndex, nativeResource, nullptr);
            }

            break;
        }
        case RenderDescriptorRangeType::CONSTANT_BUFFER:
        case RenderDescriptorRangeType::FORMATTED_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_FORMATTED_BUFFER:
        case RenderDescriptorRangeType::STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::BYTE_ADDRESS_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_STRUCTURED_BUFFER:
        case RenderDescriptorRangeType::READ_WRITE_BYTE_ADDRESS_BUFFER:
        case RenderDescriptorRangeType::SAMPLER:
        case RenderDescriptorRangeType::ACCELERATION_STRUCTURE:
            assert(false && "Incompatible descriptor type.");
            break;
        default:
            assert(false && "Unknown descriptor type.");
            break;
        }
    }

    void D3D12DescriptorSet::setSampler(uint32_t descriptorIndex, const RenderSampler *sampler) {
        if (sampler != nullptr) {
            const D3D12Sampler *interfaceSampler = static_cast<const D3D12Sampler *>(sampler);
            uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
            uint32_t descriptorIndexRelative = (descriptorIndex - descriptorIndexClamped);
            uint32_t descriptorHeapIndex = descriptorHeapIndices[descriptorIndexClamped];
            const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->samplerHeapAllocator->getCPUHandleAt(samplerAllocation.offset + descriptorHeapIndex + descriptorIndexRelative);
            device->d3d->CreateSampler(&interfaceSampler->samplerDesc, cpuHandle);
        }
    }

    void D3D12DescriptorSet::setAccelerationStructure(uint32_t descriptorIndex, const RenderAccelerationStructure *accelerationStructure) {
        const D3D12AccelerationStructure *interfaceAccelerationStructure = static_cast<const D3D12AccelerationStructure *>(accelerationStructure);
        uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
        RenderDescriptorRangeType descriptorType = descriptorTypes[descriptorIndexClamped];
        assert((descriptorType == RenderDescriptorRangeType::ACCELERATION_STRUCTURE) && "Incompatible descriptor type.");

        if (interfaceAccelerationStructure != nullptr) {
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
            srvDesc.RaytracingAccelerationStructure.Location = interfaceAccelerationStructure->buffer->d3d->GetGPUVirtualAddress();
            setSRV(descriptorIndex, nullptr, &srvDesc);
        }
        else {
            setSRV(descriptorIndex, nullptr, nullptr);
        }
    }
    
    void D3D12DescriptorSet::setSRV(uint32_t descriptorIndex, ID3D12Resource *resource, const D3D12_SHADER_RESOURCE_VIEW_DESC *viewDesc) {
        if ((resource != nullptr) || (viewDesc != nullptr)) {
            uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
            uint32_t descriptorIndexRelative = (descriptorIndex - descriptorIndexClamped);
            uint32_t descriptorHeapIndex = descriptorHeapIndices[descriptorIndexClamped];
            const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->viewHeapAllocator->getCPUHandleAt(viewAllocation.offset + descriptorHeapIndex + descriptorIndexRelative);
            device->d3d->CreateShaderResourceView(resource, viewDesc, cpuHandle);
        }
    }

    void D3D12DescriptorSet::setUAV(uint32_t descriptorIndex, ID3D12Resource *resource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *viewDesc) {
        if ((resource != nullptr) || (viewDesc != nullptr)) {
            uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
            uint32_t descriptorIndexRelative = (descriptorIndex - descriptorIndexClamped);
            uint32_t descriptorHeapIndex = descriptorHeapIndices[descriptorIndexClamped];
            const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->viewHeapAllocator->getCPUHandleAt(viewAllocation.offset + descriptorHeapIndex + descriptorIndexRelative);
            device->d3d->CreateUnorderedAccessView(resource, nullptr, viewDesc, cpuHandle);
        }
    }

    void D3D12DescriptorSet::setCBV(uint32_t descriptorIndex, ID3D12Resource *resource, uint64_t bufferSize) {
        if (resource != nullptr) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
            viewDesc.BufferLocation = resource->GetGPUVirtualAddress();
            viewDesc.SizeInBytes = UINT(roundUp(bufferSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));

            uint32_t descriptorIndexClamped = std::min(descriptorIndex, descriptorTypeMaxIndex);
            uint32_t descriptorIndexRelative = (descriptorIndex - descriptorIndexClamped);
            uint32_t descriptorHeapIndex = descriptorHeapIndices[descriptorIndexClamped];
            const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->viewHeapAllocator->getCPUHandleAt(viewAllocation.offset + descriptorHeapIndex + descriptorIndexRelative);
            device->d3d->CreateConstantBufferView(&viewDesc, cpuHandle);
        }
    }

    // D3D12SwapChain

    D3D12SwapChain::D3D12SwapChain(D3D12CommandQueue *commandQueue, RenderWindow renderWindow, uint32_t textureCount, RenderFormat format) {
        assert(commandQueue != nullptr);
        assert(renderWindow != 0);

        this->commandQueue = commandQueue;
        this->renderWindow = renderWindow;
        this->textureCount = textureCount;
        this->format = format;
        
        // Store the native format representation.
        nativeFormat = toDXGI(format);

        getWindowSize(width, height);
        
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.BufferCount = textureCount;
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = nativeFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

        IDXGISwapChain1 *swapChain1;
        IDXGIFactory4 *dxgiFactory = commandQueue->device->renderInterface->dxgiFactory;
        HRESULT res = dxgiFactory->CreateSwapChainForHwnd(commandQueue->d3d, renderWindow, &swapChainDesc, nullptr, nullptr, &swapChain1);
        if (FAILED(res)) {
            fprintf(stderr, "CreateSwapChainForHwnd failed with error code 0x%lX.\n", res);
            return;
        }

        res = dxgiFactory->MakeWindowAssociation(renderWindow, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(res)) {
            fprintf(stderr, "MakeWindowAssociation failed with error code 0x%lX.\n", res);
            return;
        }

        d3d = static_cast<IDXGISwapChain3 *>(swapChain1);
        d3d->SetMaximumFrameLatency(1);
        waitableObject = d3d->GetFrameLatencyWaitableObject();

        textures.resize(textureCount);

        for (uint32_t i = 0; i < textureCount; i++) {
            textures[i].device = commandQueue->device;
            textures[i].desc.dimension = RenderTextureDimension::TEXTURE_2D;
            textures[i].desc.format = format;
            textures[i].desc.depth = 1;
            textures[i].desc.mipLevels = 1;
            textures[i].desc.arraySize = 1;
            textures[i].desc.flags = RenderTextureFlag::RENDER_TARGET;
        }

        setTextures();
    }

    D3D12SwapChain::~D3D12SwapChain() {
        for (uint32_t i = 0; i < textureCount; i++) {
            textures[i].releaseTargetHeap();

            if (textures[i].d3d != nullptr) {
                textures[i].d3d->Release();
                textures[i].d3d = nullptr;
            }
        }

        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    bool D3D12SwapChain::present(uint32_t textureIndex, RenderCommandSemaphore **waitSemaphores, uint32_t waitSemaphoreCount) {
        if (waitableObject != NULL) {
            while (WaitForSingleObjectEx(waitableObject, 0, FALSE));
        }

        UINT syncInterval = vsyncEnabled ? 1 : 0;
        HRESULT res = d3d->Present(syncInterval, 0);
        return SUCCEEDED(res);
    }

    bool D3D12SwapChain::resize() {
        getWindowSize(width, height);

        // Don't resize the swap chain at all if the window doesn't have a valid size.
        if ((width == 0) || (height == 0)) {
            return false;
        }

        for (uint32_t i = 0; i < textureCount; i++) {
            textures[i].releaseTargetHeap();
            textures[i].d3d->Release();
            textures[i].d3d = nullptr;
        }

        HRESULT res = d3d->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        if (FAILED(res)) {
            fprintf(stderr, "ResizeBuffers failed with error code 0x%lX.\n", res);
            return false;
        }

        setTextures();
        return true;
    }

    bool D3D12SwapChain::needsResize() const {
        uint32_t windowWidth, windowHeight;
        getWindowSize(windowWidth, windowHeight);
        return (d3d == nullptr) || (windowWidth != width) || (windowHeight != height);
    }

    void D3D12SwapChain::setVsyncEnabled(bool vsyncEnabled) {
        this->vsyncEnabled = vsyncEnabled;
    }

    bool D3D12SwapChain::isVsyncEnabled() const {
        return vsyncEnabled;
    }

    uint32_t D3D12SwapChain::getWidth() const {
        return width;
    }

    uint32_t D3D12SwapChain::getHeight() const {
        return height;
    }

    void D3D12SwapChain::getWindowSize(uint32_t &dstWidth, uint32_t &dstHeight) const {
        RECT rect;
        GetClientRect(renderWindow, &rect);
        dstWidth = rect.right - rect.left;
        dstHeight = rect.bottom - rect.top;
    }

    void D3D12SwapChain::setTextures() {
        assert(textureCount == textures.size());

        for (uint32_t i = 0; i < textureCount; i++) {
            d3d->GetBuffer(i, IID_PPV_ARGS(&textures[i].d3d));

            textures[i].desc.width = width;
            textures[i].desc.height = height;
            textures[i].resourceStates = D3D12_RESOURCE_STATE_PRESENT;
            textures[i].layout = RenderTextureLayout::PRESENT;
            textures[i].createRenderTargetHeap();
        }
    }

    RenderTexture *D3D12SwapChain::getTexture(uint32_t textureIndex) {
        return &textures[textureIndex];
    }

    uint32_t D3D12SwapChain::getTextureCount() const {
        return textureCount;
    }

    bool D3D12SwapChain::acquireTexture(RenderCommandSemaphore *signalSemaphore, uint32_t *textureIndex) {
        assert(textureIndex != nullptr);
        *textureIndex = d3d->GetCurrentBackBufferIndex();
        return true;
    }

    RenderWindow D3D12SwapChain::getWindow() const {
        return renderWindow;
    }

    bool D3D12SwapChain::isEmpty() const {
        return (d3d == nullptr) || (width == 0) || (height == 0);
    }

    uint32_t D3D12SwapChain::getRefreshRate() const {
        return 0;
    }

    // D3D12Framebuffer
    
    D3D12Framebuffer::D3D12Framebuffer(D3D12Device *device, const RenderFramebufferDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        
        if (desc.colorAttachmentsCount > 0) {
            for (uint32_t i = 0; i < desc.colorAttachmentsCount; i++) {
                const D3D12Texture *interfaceTexture = static_cast<const D3D12Texture *>(desc.colorAttachments[i]);
                assert((interfaceTexture->desc.flags & RenderTextureFlag::RENDER_TARGET) && "Color attachment must be a render target.");
                colorTargets.emplace_back(interfaceTexture);
                colorHandles.emplace_back(device->colorTargetHeapAllocator->getCPUHandleAt(interfaceTexture->targetAllocatorOffset));

                if (i == 0) {
                    width = interfaceTexture->desc.width;
                    height = interfaceTexture->desc.height;
                }
            }
        }

        if (desc.depthAttachment != nullptr) {
            const D3D12Texture *interfaceTexture = static_cast<const D3D12Texture *>(desc.depthAttachment);
            assert((interfaceTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) && "Depth attachment must be a depth target.");
            depthTarget = interfaceTexture;

            // The read-only handle is on the second slot on the DSV heap.
            if (desc.depthAttachmentReadOnly) {
                depthHandle = device->depthTargetHeapAllocator->getCPUHandleAt(interfaceTexture->targetAllocatorOffset + 1);
            }
            else {
                depthHandle = device->depthTargetHeapAllocator->getCPUHandleAt(interfaceTexture->targetAllocatorOffset);
            }

            if (desc.colorAttachmentsCount == 0) {
                width = interfaceTexture->desc.width;
                height = interfaceTexture->desc.height;
            }
        }
    }

    D3D12Framebuffer::~D3D12Framebuffer() { }

    uint32_t D3D12Framebuffer::getWidth() const {
        return width;
    }

    uint32_t D3D12Framebuffer::getHeight() const {
        return height;
    }

    // D3D12CommandList

    D3D12CommandList::D3D12CommandList(D3D12Device *device, RenderCommandListType type) {
        assert(device != nullptr);

        this->device = device;
        this->type = type;

        D3D12_COMMAND_LIST_TYPE commandListType;
        switch (type) {
        case RenderCommandListType::DIRECT:
            commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case RenderCommandListType::COMPUTE:
            commandListType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case RenderCommandListType::COPY:
            commandListType = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        default:
            assert(false && "Unknown command list type.");
            return;
        }

        HRESULT res = device->d3d->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator));
        if (FAILED(res)) {
            fprintf(stderr, "CreateCommandAllocator failed with error code 0x%lX.\n", res);
            return;
        }

        res = device->d3d->CreateCommandList(0, commandListType, commandAllocator, nullptr, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateCommandList failed with error code 0x%lX.\n", res);
            return;
        }

        d3d->Close();
    }

    D3D12CommandList::~D3D12CommandList() {
        if (d3d != nullptr) {
            d3d->Release();
        }

        if (commandAllocator != nullptr) {
            commandAllocator->Release();
        }
    }

    void D3D12CommandList::begin() {
        assert(!open);

        commandAllocator->Reset();
        d3d->Reset(commandAllocator, nullptr);
        open = true;
    }

    void D3D12CommandList::end() {
        assert(open);

        // It's required to reset the sample positions before the command list ends.
        resetSamplePositions();

        d3d->Close();
        open = false;
        targetFramebuffer = nullptr;
        targetFramebufferSamplePositionsSet = false;
        activeComputePipelineLayout = nullptr;
        activeGraphicsPipelineLayout = nullptr;
        activeGraphicsPipeline = nullptr;
        activeTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        descriptorHeapsSet = false;
    }
    
    void D3D12CommandList::barriers(RenderBarrierStages stages, const RenderBufferBarrier *bufferBarriers, uint32_t bufferBarriersCount, const RenderTextureBarrier *textureBarriers, uint32_t textureBarriersCount) {
        thread_local std::vector<D3D12_RESOURCE_BARRIER> barrierVector;
        barrierVector.clear();
        
        auto makeBarrier = [&](ID3D12Resource *resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, bool supportsUAV, D3D12_RESOURCE_BARRIER &resourceBarrier) {
            resourceBarrier = {};

            if (stateBefore != stateAfter) {
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                resourceBarrier.Transition.StateBefore = stateBefore;
                resourceBarrier.Transition.StateAfter = stateAfter;
                resourceBarrier.Transition.pResource = resource;
                resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                return true;
            }
            else if (supportsUAV) {
                resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                resourceBarrier.UAV.pResource = resource;
                return true;
            }
            else {
                return false;
            }
        };

        D3D12_RESOURCE_BARRIER resourceBarrier;
        const RenderBufferFlags bufferUAVMask = RenderBufferFlag::UNORDERED_ACCESS | RenderBufferFlag::ACCELERATION_STRUCTURE;
        for (uint32_t i = 0; i < bufferBarriersCount; i++) {
            const RenderBufferBarrier &bufferBarrier = bufferBarriers[i];
            D3D12Buffer *interfaceBuffer = static_cast<D3D12Buffer *>(bufferBarrier.buffer);
            D3D12_RESOURCE_STATES stateBefore = interfaceBuffer->resourceStates;
            D3D12_RESOURCE_STATES stateAfter = toBufferState(stages, bufferBarrier.accessBits, interfaceBuffer->desc.flags);
            if (makeBarrier(interfaceBuffer->d3d, stateBefore, stateAfter, interfaceBuffer->desc.flags & bufferUAVMask, resourceBarrier)) {
                barrierVector.emplace_back(resourceBarrier);
            }

            interfaceBuffer->resourceStates = stateAfter;
        }
        
        bool resetSamplePositionsRequired = false;
        for (uint32_t i = 0; i < textureBarriersCount; i++) {
            const RenderTextureBarrier &textureBarrier = textureBarriers[i];
            D3D12Texture *interfaceTexture = static_cast<D3D12Texture *>(textureBarrier.texture);
            D3D12_RESOURCE_STATES stateBefore = interfaceTexture->resourceStates;
            D3D12_RESOURCE_STATES stateAfter = toTextureState(stages, textureBarrier.layout, interfaceTexture->desc.flags);
            bool madeBarrier = makeBarrier(interfaceTexture->d3d, stateBefore, stateAfter, interfaceTexture->desc.flags & RenderTextureFlag::UNORDERED_ACCESS, resourceBarrier);
            interfaceTexture->resourceStates = stateAfter;
            interfaceTexture->layout = textureBarrier.layout;
            if (!madeBarrier) {
                continue;
            }
            
            // MSAA Depth targets with multisampling require separate barriers.
            const bool msaaDepthTarget = (interfaceTexture->desc.flags & RenderTextureFlag::DEPTH_TARGET) && (interfaceTexture->desc.multisampling.sampleCount > 1);
            if (msaaDepthTarget && interfaceTexture->desc.multisampling.sampleLocationsEnabled) {
                setSamplePositions(interfaceTexture);
                d3d->ResourceBarrier(1, &resourceBarrier);
                resetSamplePositionsRequired = true;
            }
            else {
                barrierVector.emplace_back(resourceBarrier);
            }
        }

        if (resetSamplePositionsRequired) {
            resetSamplePositions();
        }
        
        if (!barrierVector.empty()) {
            d3d->ResourceBarrier(UINT(barrierVector.size()), barrierVector.data());
        }
    }

    void D3D12CommandList::dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ) {
        d3d->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }

    void D3D12CommandList::traceRays(uint32_t width, uint32_t height, uint32_t depth, RenderBufferReference shaderBindingTable, const RenderShaderBindingGroupsInfo &shaderBindingGroupsInfo) {
        const D3D12Buffer *interfaceBuffer = static_cast<const D3D12Buffer *>(shaderBindingTable.ref);
        assert(interfaceBuffer != nullptr);
        assert((interfaceBuffer->desc.flags & RenderBufferFlag::SHADER_BINDING_TABLE) && "Buffer must allow being used as a shader binding table.");

        D3D12_GPU_VIRTUAL_ADDRESS tableAddress = interfaceBuffer->d3d->GetGPUVirtualAddress() + shaderBindingTable.offset;
        const RenderShaderBindingGroupInfo &rayGen = shaderBindingGroupsInfo.rayGen;
        const RenderShaderBindingGroupInfo &miss = shaderBindingGroupsInfo.miss;
        const RenderShaderBindingGroupInfo &hitGroup = shaderBindingGroupsInfo.hitGroup;
        const RenderShaderBindingGroupInfo &callable = shaderBindingGroupsInfo.callable;
        D3D12_DISPATCH_RAYS_DESC desc;
        desc.RayGenerationShaderRecord.StartAddress = (rayGen.size > 0) ? (tableAddress + rayGen.offset + rayGen.startIndex * rayGen.stride) : 0;
        desc.RayGenerationShaderRecord.SizeInBytes = rayGen.size;
        desc.MissShaderTable.StartAddress = (miss.size > 0) ? (tableAddress + miss.offset + miss.startIndex * miss.stride) : 0;
        desc.MissShaderTable.SizeInBytes = miss.size;
        desc.MissShaderTable.StrideInBytes = miss.stride;
        desc.HitGroupTable.StartAddress = (hitGroup.size > 0) ? (tableAddress + hitGroup.offset + hitGroup.startIndex * hitGroup.stride) : 0;
        desc.HitGroupTable.SizeInBytes = hitGroup.size;
        desc.HitGroupTable.StrideInBytes = hitGroup.stride;
        desc.CallableShaderTable.StartAddress = (callable.size > 0) ? (tableAddress + callable.offset + callable.startIndex * callable.stride) : 0;
        desc.CallableShaderTable.SizeInBytes = callable.size;
        desc.CallableShaderTable.StrideInBytes = callable.stride;
        desc.Width = width;
        desc.Height = height;
        desc.Depth = depth;
        d3d->DispatchRays(&desc);
    }
    
    void D3D12CommandList::drawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation) {
        checkTopology();
        checkFramebufferSamplePositions();
        d3d->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
    }

    void D3D12CommandList::drawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation) {
        checkTopology();
        checkFramebufferSamplePositions();
        d3d->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
    }

    void D3D12CommandList::setPipeline(const RenderPipeline *pipeline) {
        assert(pipeline != nullptr);

        const D3D12Pipeline *interfacePipeline = static_cast<const D3D12Pipeline *>(pipeline);
        switch (interfacePipeline->type) {
        case D3D12Pipeline::Type::Compute: {
            const D3D12ComputePipeline *computePipeline = static_cast<const D3D12ComputePipeline *>(interfacePipeline);
            d3d->SetPipelineState(computePipeline->d3d);
            break;
        }
        case D3D12Pipeline::Type::Graphics: {
            const D3D12GraphicsPipeline *graphicsPipeline = static_cast<const D3D12GraphicsPipeline *>(interfacePipeline);
            d3d->SetPipelineState(graphicsPipeline->d3d);
            activeGraphicsPipeline = graphicsPipeline;
            break;
        }
        case D3D12Pipeline::Type::Raytracing: {
            const D3D12RaytracingPipeline *raytracingPipeline = static_cast<const D3D12RaytracingPipeline *>(interfacePipeline);
            d3d->SetPipelineState1(raytracingPipeline->stateObject);
            break;
        }
        default:
            assert(false && "Unknown pipeline type.");
            break;
        }
    }

    void D3D12CommandList::setComputePipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        assert(pipelineLayout != nullptr);

        const D3D12PipelineLayout *interfacePipelineLayout = static_cast<const D3D12PipelineLayout *>(pipelineLayout);
        d3d->SetComputeRootSignature(interfacePipelineLayout->rootSignature);
        activeComputePipelineLayout = interfacePipelineLayout;
    }

    void D3D12CommandList::setComputePushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        assert(activeComputePipelineLayout != nullptr);
        assert(rangeIndex < activeComputePipelineLayout->pushConstantRanges.size());

        const RenderPushConstantRange &range = activeComputePipelineLayout->pushConstantRanges[rangeIndex];
        assert((range.offset == 0) && "Offset behavior should be verified when compared to Vulkan.");

        if (size == 0) {
            size = range.size;
        }

        d3d->SetComputeRoot32BitConstants(rangeIndex, (size + sizeof(uint32_t) - 1) / sizeof(uint32_t), data, (offset + sizeof(uint32_t) - 1) / sizeof(uint32_t));
    }

    void D3D12CommandList::setComputeDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setDescriptorSet(activeComputePipelineLayout, descriptorSet, setIndex, true);
    }

    void D3D12CommandList::setGraphicsPipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        assert(pipelineLayout != nullptr);

        const D3D12PipelineLayout *interfacePipelineLayout = static_cast<const D3D12PipelineLayout *>(pipelineLayout);
        d3d->SetGraphicsRootSignature(interfacePipelineLayout->rootSignature);
        activeGraphicsPipelineLayout = interfacePipelineLayout;
    }

    void D3D12CommandList::setGraphicsPushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        assert(activeGraphicsPipelineLayout != nullptr);
        assert(rangeIndex < activeGraphicsPipelineLayout->pushConstantRanges.size());

        const RenderPushConstantRange &range = activeGraphicsPipelineLayout->pushConstantRanges[rangeIndex];
        assert((range.offset == 0) && "Offset behavior should be verified when compared to Vulkan.");

        if (size == 0) {
            size = range.size;
        }

        d3d->SetGraphicsRoot32BitConstants(rangeIndex, (size + sizeof(uint32_t) - 1) / sizeof(uint32_t), data, (offset + sizeof(uint32_t) - 1) / sizeof(uint32_t));
    }

    void D3D12CommandList::setGraphicsDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setDescriptorSet(activeGraphicsPipelineLayout, descriptorSet, setIndex, false);
    }

    void D3D12CommandList::setGraphicsRootDescriptor(RenderBufferReference bufferReference, uint32_t rootDescriptorIndex) {
        setRootDescriptor(activeGraphicsPipelineLayout, bufferReference, rootDescriptorIndex, false);
    }

    void D3D12CommandList::setRaytracingPipelineLayout(const RenderPipelineLayout *pipelineLayout) {
        setComputePipelineLayout(pipelineLayout);
    }

    void D3D12CommandList::setRaytracingPushConstants(uint32_t rangeIndex, const void *data, uint32_t offset, uint32_t size) {
        setComputePushConstants(rangeIndex, data, offset, size);
    }

    void D3D12CommandList::setRaytracingDescriptorSet(RenderDescriptorSet *descriptorSet, uint32_t setIndex) {
        setComputeDescriptorSet(descriptorSet, setIndex);
    }

    void D3D12CommandList::setIndexBuffer(const RenderIndexBufferView *view) {
        if (view != nullptr) {
            const D3D12Buffer *interfaceBuffer = static_cast<const D3D12Buffer *>(view->buffer.ref);
            D3D12_INDEX_BUFFER_VIEW bufferView;
            bufferView.BufferLocation = (interfaceBuffer != nullptr) ? (interfaceBuffer->d3d->GetGPUVirtualAddress() + view->buffer.offset) : 0;
            bufferView.Format = toDXGI(view->format);
            bufferView.SizeInBytes = view->size;
            d3d->IASetIndexBuffer(&bufferView);
        }
        else {
            d3d->IASetIndexBuffer(nullptr);
        }
    }
    
    void D3D12CommandList::setVertexBuffers(uint32_t startSlot, const RenderVertexBufferView *views, uint32_t viewCount, const RenderInputSlot *inputSlots) {
        if (views != nullptr) {
            assert(inputSlots != nullptr);

            thread_local std::vector<D3D12_VERTEX_BUFFER_VIEW> bufferViewVector;
            bufferViewVector.resize(viewCount, {});
            for (uint32_t i = 0; i < viewCount; i++) {
                const RenderVertexBufferView &renderView = views[i];
                const D3D12Buffer *interfaceBuffer = static_cast<const D3D12Buffer *>(renderView.buffer.ref);
                bufferViewVector[i].BufferLocation = (interfaceBuffer != nullptr) ? (interfaceBuffer->d3d->GetGPUVirtualAddress() + renderView.buffer.offset) : 0;
                bufferViewVector[i].SizeInBytes = renderView.size;

                bool slotFound = false;
                for (uint32_t j = 0; j < viewCount; j++) {
                    if (inputSlots[j].index == (startSlot + i)) {
                        bufferViewVector[i].StrideInBytes = inputSlots[j].stride;
                        slotFound = true;
                        break;
                    }
                }

                assert(slotFound && "Input slots must contain a slot with the same index as the view.");
            }

            d3d->IASetVertexBuffers(startSlot, viewCount, bufferViewVector.data());
        }
        else {
            d3d->IASetVertexBuffers(startSlot, viewCount, nullptr);
        }
    }

    void D3D12CommandList::setViewports(const RenderViewport *viewports, uint32_t count) {
        if (count > 1) {
            thread_local std::vector<D3D12_VIEWPORT> viewportVector;
            viewportVector.clear();
            for (uint32_t i = 0; i < count; i++) {
                viewportVector.emplace_back(D3D12_VIEWPORT{ viewports[i].x, viewports[i].y, viewports[i].width, viewports[i].height, viewports[i].minDepth, viewports[i].maxDepth });
            }

            if (!viewportVector.empty()) {
                d3d->RSSetViewports(UINT(viewportVector.size()), viewportVector.data());
            }
        }
        else {
            // Single element fast path.
            D3D12_VIEWPORT viewport = D3D12_VIEWPORT{ viewports[0].x, viewports[0].y, viewports[0].width, viewports[0].height, viewports[0].minDepth, viewports[0].maxDepth };
            d3d->RSSetViewports(1, &viewport);
        }
    }

    void D3D12CommandList::setScissors(const RenderRect *scissorRects, uint32_t count) {
        if (count > 1) {
            thread_local std::vector<D3D12_RECT> rectVector;
            rectVector.clear();
            for (uint32_t i = 0; i < count; i++) {
                rectVector.emplace_back(D3D12_RECT{ scissorRects[i].left, scissorRects[i].top, scissorRects[i].right, scissorRects[i].bottom });
            }

            if (!rectVector.empty()) {
                d3d->RSSetScissorRects(UINT(rectVector.size()), rectVector.data());
            }
        }
        else {
            // Single element fast path.
            D3D12_RECT scissor = D3D12_RECT{ scissorRects[0].left, scissorRects[0].top, scissorRects[0].right, scissorRects[0].bottom };
            d3d->RSSetScissorRects(1, &scissor);
        }
    }
    
    void D3D12CommandList::setFramebuffer(const RenderFramebuffer *framebuffer) {
        if (framebuffer != nullptr) {
            const D3D12Framebuffer *interfaceFramebuffer = static_cast<const D3D12Framebuffer *>(framebuffer);
            for (const D3D12Texture *target : interfaceFramebuffer->colorTargets) {
                assert((target->layout == RenderTextureLayout::COLOR_WRITE) && "Color targets must be in color write layout when setting the framebuffer.");
            }
            
            if (interfaceFramebuffer->depthTarget != nullptr) {
                const bool depthReadLayout = (interfaceFramebuffer->depthTarget->layout == RenderTextureLayout::DEPTH_READ);
                const bool depthWriteLayout = (interfaceFramebuffer->depthTarget->layout == RenderTextureLayout::DEPTH_WRITE);
                assert((depthReadLayout || depthWriteLayout) && "Depth target must be in depth read or write layout when setting the framebuffer.");
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE *colorDescriptors = !interfaceFramebuffer->colorHandles.empty() ? interfaceFramebuffer->colorHandles.data() : nullptr;
            const D3D12_CPU_DESCRIPTOR_HANDLE *depthDescriptor = (interfaceFramebuffer->depthHandle.ptr != 0) ? &interfaceFramebuffer->depthHandle : nullptr;
            d3d->OMSetRenderTargets(UINT(interfaceFramebuffer->colorHandles.size()), colorDescriptors, false, depthDescriptor);
            targetFramebuffer = interfaceFramebuffer;
            targetFramebufferSamplePositionsSet = false;
        }
        else {
            d3d->OMSetRenderTargets(0, nullptr, false, nullptr);
            targetFramebuffer = nullptr;
        }
    }

    void D3D12CommandList::clearColor(uint32_t attachmentIndex, RenderColor colorValue, const RenderRect *clearRects, uint32_t clearRectsCount) {
        assert(targetFramebuffer != nullptr);
        assert(attachmentIndex < targetFramebuffer->colorTargets.size());
        assert((clearRectsCount == 0) || (clearRects != nullptr));

        checkFramebufferSamplePositions();

        thread_local std::vector<D3D12_RECT> rectVector;
        if (clearRectsCount > 0) {
            rectVector.clear();
            for (uint32_t i = 0; i < clearRectsCount; i++) {
                rectVector.emplace_back(D3D12_RECT{ clearRects[i].left, clearRects[i].top, clearRects[i].right, clearRects[i].bottom });
            }
        }

        d3d->ClearRenderTargetView(targetFramebuffer->colorHandles[attachmentIndex], colorValue.rgba, clearRectsCount, (clearRectsCount > 0) ? rectVector.data() : nullptr);
    }

    void D3D12CommandList::clearDepth(bool clearDepth, float depthValue, const RenderRect *clearRects, uint32_t clearRectsCount) {
        assert(targetFramebuffer != nullptr);
        assert(targetFramebuffer->depthHandle.ptr != 0);
        assert((clearRectsCount == 0) || (clearRects != nullptr));

        checkFramebufferSamplePositions();

        thread_local std::vector<D3D12_RECT> rectVector;
        if (clearRectsCount > 0) {
            rectVector.clear();
            for (uint32_t i = 0; i < clearRectsCount; i++) {
                rectVector.emplace_back(D3D12_RECT{ clearRects[i].left, clearRects[i].top, clearRects[i].right, clearRects[i].bottom });
            }
        }

        D3D12_CLEAR_FLAGS clearFlags = {};
        clearFlags |= clearDepth ? D3D12_CLEAR_FLAG_DEPTH : D3D12_CLEAR_FLAGS(0);
        d3d->ClearDepthStencilView(targetFramebuffer->depthHandle, clearFlags, depthValue, 0, clearRectsCount, (clearRectsCount > 0) ? rectVector.data() : nullptr);
    }

    void D3D12CommandList::copyBufferRegion(RenderBufferReference dstBuffer, RenderBufferReference srcBuffer, uint64_t size) {
        assert(dstBuffer.ref != nullptr);
        assert(srcBuffer.ref != nullptr);

        const D3D12Buffer *interfaceDstBuffer = static_cast<const D3D12Buffer *>(dstBuffer.ref);
        const D3D12Buffer *interfaceSrcBuffer = static_cast<const D3D12Buffer *>(srcBuffer.ref);
        d3d->CopyBufferRegion(interfaceDstBuffer->d3d, dstBuffer.offset, interfaceSrcBuffer->d3d, srcBuffer.offset, size);
    }

    void D3D12CommandList::copyTextureRegion(const RenderTextureCopyLocation &dstLocation, const RenderTextureCopyLocation &srcLocation, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const RenderBox *srcBox) {
        D3D12_BOX copyBox;
        if (srcBox != nullptr) {
            copyBox.left = srcBox->left;
            copyBox.top = srcBox->top;
            copyBox.front = srcBox->front;
            copyBox.right = srcBox->right;
            copyBox.bottom = srcBox->bottom;
            copyBox.back = srcBox->back;
        }

        const D3D12_TEXTURE_COPY_LOCATION copyDstLocation = toD3D12(dstLocation);
        const D3D12_TEXTURE_COPY_LOCATION copySrcLocation = toD3D12(srcLocation);
        setSamplePositions(dstLocation.texture);
        d3d->CopyTextureRegion(&copyDstLocation, dstX, dstY, dstZ, &copySrcLocation, (srcBox != nullptr) ? &copyBox : nullptr);
        resetSamplePositions();
    }

    void D3D12CommandList::copyBuffer(const RenderBuffer *dstBuffer, const RenderBuffer *srcBuffer) {
        assert(dstBuffer != nullptr);
        assert(srcBuffer != nullptr);

        const D3D12Buffer *interfaceDstBuffer = static_cast<const D3D12Buffer *>(dstBuffer);
        const D3D12Buffer *interfaceSrcBuffer = static_cast<const D3D12Buffer *>(srcBuffer);
        d3d->CopyResource(interfaceDstBuffer->d3d, interfaceSrcBuffer->d3d);
    }

    void D3D12CommandList::copyTexture(const RenderTexture *dstTexture, const RenderTexture *srcTexture) {
        assert(dstTexture != nullptr);
        assert(srcTexture != nullptr);

        const D3D12Texture *interfaceDstTexture = static_cast<const D3D12Texture *>(dstTexture);
        const D3D12Texture *interfaceSrcTexture = static_cast<const D3D12Texture *>(srcTexture);
        setSamplePositions(interfaceDstTexture);
        d3d->CopyResource(interfaceDstTexture->d3d, interfaceSrcTexture->d3d);
        resetSamplePositions();
    }

    void D3D12CommandList::resolveTexture(const RenderTexture *dstTexture, const RenderTexture *srcTexture) {
        assert(dstTexture != nullptr);
        assert(srcTexture != nullptr);

        const D3D12Texture *interfaceDstTexture = static_cast<const D3D12Texture *>(dstTexture);
        const D3D12Texture *interfaceSrcTexture = static_cast<const D3D12Texture *>(srcTexture);
        setSamplePositions(interfaceDstTexture);
        d3d->ResolveSubresource(interfaceDstTexture->d3d, 0, interfaceSrcTexture->d3d, 0, toDXGI(interfaceDstTexture->desc.format));
        resetSamplePositions();
    }

    void D3D12CommandList::resolveTextureRegion(const RenderTexture *dstTexture, uint32_t dstX, uint32_t dstY, const RenderTexture *srcTexture, const RenderRect *srcRect) {
        assert(dstTexture != nullptr);
        assert(srcTexture != nullptr);

        const D3D12Texture *interfaceDstTexture = static_cast<const D3D12Texture *>(dstTexture);
        const D3D12Texture *interfaceSrcTexture = static_cast<const D3D12Texture *>(srcTexture);
        D3D12_RECT rect;
        if (srcRect != nullptr) {
            rect.left = srcRect->left;
            rect.top = srcRect->top;
            rect.right = srcRect->right;
            rect.bottom = srcRect->bottom;
        }

        setSamplePositions(interfaceDstTexture);
        d3d->ResolveSubresourceRegion(interfaceDstTexture->d3d, 0, dstX, dstY, interfaceSrcTexture->d3d, 0, (srcRect != nullptr) ? &rect : nullptr, toDXGI(interfaceDstTexture->desc.format), D3D12_RESOLVE_MODE_AVERAGE);
        resetSamplePositions();
    }

    void D3D12CommandList::buildBottomLevelAS(const RenderAccelerationStructure *dstAccelerationStructure, RenderBufferReference scratchBuffer, const RenderBottomLevelASBuildInfo &buildInfo) {
        assert(dstAccelerationStructure != nullptr);
        assert(scratchBuffer.ref != nullptr);

        const D3D12AccelerationStructure *interfaceAccelerationStructure = static_cast<const D3D12AccelerationStructure *>(dstAccelerationStructure);
        assert(interfaceAccelerationStructure->type == RenderAccelerationStructureType::BOTTOM_LEVEL);
        
        const D3D12Buffer *interfaceScratchBuffer = static_cast<const D3D12Buffer *>(scratchBuffer.ref);
        assert((interfaceScratchBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH) && "Scratch buffer must be allowed.");

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
        buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        buildDesc.Inputs.NumDescs = buildInfo.meshCount;
        buildDesc.Inputs.pGeometryDescs = reinterpret_cast<const D3D12_RAYTRACING_GEOMETRY_DESC *>(buildInfo.buildData.data());
        buildDesc.Inputs.Flags = toRTASBuildFlags(buildInfo.preferFastBuild, buildInfo.preferFastTrace);
        buildDesc.DestAccelerationStructureData = interfaceAccelerationStructure->buffer->d3d->GetGPUVirtualAddress() + interfaceAccelerationStructure->offset;
        buildDesc.ScratchAccelerationStructureData = interfaceScratchBuffer->d3d->GetGPUVirtualAddress() + scratchBuffer.offset;

        d3d->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
    }

    void D3D12CommandList::buildTopLevelAS(const RenderAccelerationStructure *dstAccelerationStructure, RenderBufferReference scratchBuffer, RenderBufferReference instancesBuffer, const RenderTopLevelASBuildInfo &buildInfo) {
        assert(dstAccelerationStructure != nullptr);
        assert(scratchBuffer.ref != nullptr);
        assert(instancesBuffer.ref != nullptr);

        const D3D12AccelerationStructure *interfaceAccelerationStructure = static_cast<const D3D12AccelerationStructure *>(dstAccelerationStructure);
        assert(interfaceAccelerationStructure->type == RenderAccelerationStructureType::TOP_LEVEL);;

        const D3D12Buffer *interfaceScratchBuffer = static_cast<const D3D12Buffer *>(scratchBuffer.ref);
        assert((interfaceScratchBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH) && "Scratch buffer must be allowed.");

        const D3D12Buffer *interfaceInstancesBuffer = static_cast<const D3D12Buffer *>(instancesBuffer.ref);
        assert((interfaceInstancesBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input must be allowed.");

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
        buildDesc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        buildDesc.Inputs.NumDescs = buildInfo.instanceCount;
        buildDesc.Inputs.InstanceDescs = interfaceInstancesBuffer->d3d->GetGPUVirtualAddress() + instancesBuffer.offset;
        buildDesc.Inputs.Flags = toRTASBuildFlags(buildInfo.preferFastBuild, buildInfo.preferFastTrace);
        buildDesc.DestAccelerationStructureData = interfaceAccelerationStructure->buffer->d3d->GetGPUVirtualAddress() + interfaceAccelerationStructure->offset;
        buildDesc.ScratchAccelerationStructureData = interfaceScratchBuffer->d3d->GetGPUVirtualAddress() + scratchBuffer.offset;

        d3d->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
    }

    void D3D12CommandList::discardTexture(const RenderTexture* texture) {
        const D3D12Texture* interfaceTexture = static_cast<const D3D12Texture*>(texture);
        d3d->DiscardResource(interfaceTexture->d3d, nullptr);
    }

    void D3D12CommandList::checkDescriptorHeaps() {
        if (!descriptorHeapsSet) {
            ID3D12DescriptorHeap *descriptorHeaps[] = { device->viewHeapAllocator->heap, device->samplerHeapAllocator->heap };
            d3d->SetDescriptorHeaps(std::size(descriptorHeaps), descriptorHeaps);
            descriptorHeapsSet = true;
        }
    }

    void D3D12CommandList::notifyDescriptorHeapWasChangedExternally() {
        descriptorHeapsSet = false;
    }

    void D3D12CommandList::checkTopology() {
        assert(activeGraphicsPipeline != nullptr);
        assert(activeGraphicsPipeline->type == D3D12Pipeline::Type::Graphics);

        const D3D12GraphicsPipeline *graphicsPipeline = static_cast<const D3D12GraphicsPipeline *>(activeGraphicsPipeline);
        if (activeTopology != graphicsPipeline->topology) {
            d3d->IASetPrimitiveTopology(graphicsPipeline->topology);
            activeTopology = graphicsPipeline->topology;
        }
    }
    
    void D3D12CommandList::checkFramebufferSamplePositions() {
        if (!targetFramebufferSamplePositionsSet && (targetFramebuffer != nullptr)) {
            if (targetFramebuffer->depthTarget != nullptr) {
                setSamplePositions(targetFramebuffer->depthTarget);
            }

            targetFramebufferSamplePositionsSet = true;
        }
    }
    
    void D3D12CommandList::setSamplePositions(const RenderTexture *texture) {
        assert(texture != nullptr);

        const D3D12Texture *interfaceTexture = static_cast<const D3D12Texture *>(texture);
        if (interfaceTexture->desc.multisampling.sampleLocationsEnabled) {
            thread_local std::vector<D3D12_SAMPLE_POSITION> samplePositions;
            samplePositions.resize(interfaceTexture->desc.multisampling.sampleCount);
            for (uint32_t i = 0; i < interfaceTexture->desc.multisampling.sampleCount; i++) {
                const RenderMultisamplingLocation &location = interfaceTexture->desc.multisampling.sampleLocations[i];
                samplePositions[i].X = location.x;
                samplePositions[i].Y = location.y;
            }

            d3d->SetSamplePositions(uint32_t(samplePositions.size()), 1, samplePositions.data());
            activeSamplePositions = true;
        }
        else {
            resetSamplePositions();
        }
    }

    void D3D12CommandList::resetSamplePositions() {
        if (activeSamplePositions) {
            d3d->SetSamplePositions(0, 0, nullptr);
            activeSamplePositions = false;
            targetFramebufferSamplePositionsSet = false;
        }
    }


    void D3D12CommandList::setDescriptorSet(const D3D12PipelineLayout *activePipelineLayout, RenderDescriptorSet *descriptorSet, uint32_t setIndex, bool setCompute) {
        assert(descriptorSet != nullptr);
        assert(activePipelineLayout != nullptr);
        assert(setIndex < activePipelineLayout->setCount);

        checkDescriptorHeaps();

        D3D12DescriptorSet *interfaceDescriptorSet = static_cast<D3D12DescriptorSet *>(descriptorSet);
        setRootDescriptorTable(device->viewHeapAllocator.get(), interfaceDescriptorSet->viewAllocation, activePipelineLayout->setViewRootIndices[setIndex], setCompute);
        setRootDescriptorTable(device->samplerHeapAllocator.get(), interfaceDescriptorSet->samplerAllocation, activePipelineLayout->setSamplerRootIndices[setIndex], setCompute);
    }

    void D3D12CommandList::setRootDescriptorTable(D3D12DescriptorHeapAllocator *heapAllocator, D3D12DescriptorSet::HeapAllocation &heapAllocation, uint32_t rootIndex, bool setCompute) {
        if (heapAllocation.count == 0) {
            return;
        }

        const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heapAllocator->getGPUHandleAt(heapAllocation.offset);
        if (setCompute) {
            d3d->SetComputeRootDescriptorTable(rootIndex, gpuHandle);
        }
        else {
            d3d->SetGraphicsRootDescriptorTable(rootIndex, gpuHandle);
        }
    }

    void D3D12CommandList::setRootDescriptor(const D3D12PipelineLayout* activePipelineLayout, RenderBufferReference bufferReference, uint32_t rootDescriptorIndex, bool setCompute) {
        assert(rootDescriptorIndex < activePipelineLayout->rootDescriptorRootIndicesAndTypes.size());

        D3D12_GPU_VIRTUAL_ADDRESS gpuVA = static_cast<const D3D12Buffer*>(bufferReference.ref)->d3d->GetGPUVirtualAddress() + bufferReference.offset;
        const auto& [rootParamIndex, type] = activePipelineLayout->rootDescriptorRootIndicesAndTypes[rootDescriptorIndex];

        if (setCompute) {
            switch (type) {
            case RenderRootDescriptorType::CONSTANT_BUFFER:
                d3d->SetComputeRootConstantBufferView(rootParamIndex, gpuVA);
                break;
            case RenderRootDescriptorType::SHADER_RESOURCE:
                d3d->SetComputeRootShaderResourceView(rootParamIndex, gpuVA);
                break;
            case RenderRootDescriptorType::UNORDERED_ACCESS:
                d3d->SetComputeRootUnorderedAccessView(rootParamIndex, gpuVA);
                break;
            default:
                assert(false && "Unknown root descriptor type.");
                break;
            }
        }
        else {
            switch (type) {
            case RenderRootDescriptorType::CONSTANT_BUFFER:
                d3d->SetGraphicsRootConstantBufferView(rootParamIndex, gpuVA);
                break;
            case RenderRootDescriptorType::SHADER_RESOURCE:
                d3d->SetGraphicsRootShaderResourceView(rootParamIndex, gpuVA);
                break;
            case RenderRootDescriptorType::UNORDERED_ACCESS:
                d3d->SetGraphicsRootUnorderedAccessView(rootParamIndex, gpuVA);
                break;
            default:
                assert(false && "Unknown root descriptor type.");
                break;
            }
        }
    }

    // D3D12CommandFence

    D3D12CommandFence::D3D12CommandFence(D3D12Device *device) {
        assert(device != nullptr);

        this->device = device;

        HRESULT res = device->d3d->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateFence failed with error code 0x%lX.\n", res);
            return;
        }

        fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        fenceValue = 1;
    }

    D3D12CommandFence::~D3D12CommandFence() {
        if (d3d != nullptr) {
            d3d->Release();
        }

        if (fenceEvent != 0) {
            CloseHandle(fenceEvent);
        }
    }

    // D3D12CommandSemaphore

    D3D12CommandSemaphore::D3D12CommandSemaphore(D3D12Device *device) {
        assert(device != nullptr);

        this->device = device;

        HRESULT res = device->d3d->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateFence failed with error code 0x%lX.\n", res);
            return;
        }
    }

    D3D12CommandSemaphore::~D3D12CommandSemaphore() {
        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    // D3D12CommandQueue

    D3D12CommandQueue::D3D12CommandQueue(D3D12Device *device, RenderCommandListType type) {
        assert(device != nullptr);

        this->device = device;
        this->type = type;

        D3D12_COMMAND_QUEUE_DESC queueDesc = { };
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.NodeMask = 0;

        switch (type) {
        case RenderCommandListType::DIRECT:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            break;
        case RenderCommandListType::COMPUTE:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            break;
        case RenderCommandListType::COPY:
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            break;
        default:
            assert(false && "Unknown command list type.");
            return;
        }

        HRESULT res = device->d3d->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateCommandQueue failed with error code 0x%lX.\n", res);
            return;
        }
    }

    D3D12CommandQueue::~D3D12CommandQueue() {
        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    std::unique_ptr<RenderSwapChain> D3D12CommandQueue::createSwapChain(RenderWindow renderWindow, uint32_t bufferCount, RenderFormat format) {
        return std::make_unique<D3D12SwapChain>(this, renderWindow, bufferCount, format);
    }

    void D3D12CommandQueue::executeCommandLists(const RenderCommandList **commandLists, uint32_t commandListCount, RenderCommandSemaphore **waitSemaphores, uint32_t waitSemaphoreCount, RenderCommandSemaphore **signalSemaphores, uint32_t signalSemaphoreCount, RenderCommandFence *signalFence) {
        for (uint32_t i = 0; i < waitSemaphoreCount; i++) {
            D3D12CommandSemaphore *interfaceSemaphore = static_cast<D3D12CommandSemaphore *>(waitSemaphores[i]);
            d3d->Wait(interfaceSemaphore->d3d, interfaceSemaphore->semaphoreValue);
        }

        thread_local std::vector<ID3D12CommandList *> executionVector;
        executionVector.clear();
        for (uint32_t i = 0; i < commandListCount; i++) {
            const D3D12CommandList *interfaceCommandList = static_cast<const D3D12CommandList *>(commandLists[i]);
            executionVector.emplace_back(static_cast<ID3D12CommandList *>(interfaceCommandList->d3d));
        }

        if (!executionVector.empty()) {
            d3d->ExecuteCommandLists(UINT(executionVector.size()), executionVector.data());
        }

        for (uint32_t i = 0; i < signalSemaphoreCount; i++) {
            D3D12CommandSemaphore *interfaceSemaphore = static_cast<D3D12CommandSemaphore *>(signalSemaphores[i]);
            interfaceSemaphore->semaphoreValue++;
            d3d->Signal(interfaceSemaphore->d3d, interfaceSemaphore->semaphoreValue);
        }
        
        if (signalFence != nullptr) {
            D3D12CommandFence *interfaceFence = static_cast<D3D12CommandFence *>(signalFence);
            d3d->Signal(interfaceFence->d3d, interfaceFence->fenceValue);
            interfaceFence->d3d->SetEventOnCompletion(interfaceFence->fenceValue, interfaceFence->fenceEvent);
            interfaceFence->fenceValue++;
        }
    }

    void D3D12CommandQueue::waitForCommandFence(RenderCommandFence *fence) {
        assert(fence != nullptr);

        D3D12CommandFence *interfaceFence = static_cast<D3D12CommandFence *>(fence);
        WaitForSingleObjectEx(interfaceFence->fenceEvent, INFINITE, FALSE);
    }

    // D3D12Buffer

    D3D12Buffer::D3D12Buffer(D3D12Device *device, D3D12Pool *pool, const RenderBufferDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->pool = pool;
        this->desc = desc;

        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = desc.size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        const uint32_t unorderedAccessMask = RenderBufferFlag::ACCELERATION_STRUCTURE | RenderBufferFlag::ACCELERATION_STRUCTURE_SCRATCH | RenderBufferFlag::UNORDERED_ACCESS;
        resourceDesc.Flags |= (desc.flags & unorderedAccessMask) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

        // Default to acceleration structure state if allowed.
        if ((desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE)) {
            resourceStates |= D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
        }
        
        // Resources on upload heap require generic read during creation.
        if (desc.heapType == RenderHeapType::UPLOAD) {
            resourceStates |= D3D12_RESOURCE_STATE_GENERIC_READ;
        }
        // Resources on readback heap require copy dest during creation.
        else if (desc.heapType == RenderHeapType::READBACK) {
            resourceStates |= D3D12_RESOURCE_STATE_COPY_DEST;
        }

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.Flags = desc.committed ? D3D12MA::ALLOCATION_FLAG_COMMITTED : D3D12MA::ALLOCATION_FLAG_NONE;
        allocationDesc.HeapType = toD3D12(desc.heapType);
        allocationDesc.CustomPool = (pool != nullptr) ? pool->d3d : nullptr;

        HRESULT res = device->allocator->CreateResource(&allocationDesc, &resourceDesc, resourceStates, nullptr, &allocation, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateResource failed with error code 0x%lX.\n", res);
            return;
        }
    }

    D3D12Buffer::~D3D12Buffer() {
        if (allocation != nullptr) {
            d3d->Release();
            allocation->Release();
        }
    }

    void *D3D12Buffer::map(uint32_t subresource, const RenderRange *readRange) {
        D3D12_RANGE range;
        if (readRange != nullptr) {
            range.Begin = readRange->begin;
            range.End = readRange->end;
        }

        void *outputData;
        d3d->Map(subresource, (readRange != nullptr) ? &range : nullptr, &outputData);
        return outputData;
    }

    void D3D12Buffer::unmap(uint32_t subresource, const RenderRange *writtenRange) {
        D3D12_RANGE range;
        if (writtenRange != nullptr) {
            range.Begin = writtenRange->begin;
            range.End = writtenRange->end;
        }

        d3d->Unmap(subresource, (writtenRange != nullptr) ? &range : nullptr);
    }

    std::unique_ptr<RenderBufferFormattedView> D3D12Buffer::createBufferFormattedView(RenderFormat format) {
        return std::make_unique<D3D12BufferFormattedView>(this, format);
    }

    void D3D12Buffer::setName(const std::string &name) {
        setObjectName(d3d, name);
    }

    uint64_t D3D12Buffer::getDeviceAddress() const {
        return d3d->GetGPUVirtualAddress();
    }

    // D3D12BufferFormattedView

    D3D12BufferFormattedView::D3D12BufferFormattedView(D3D12Buffer *buffer, RenderFormat format) {
        assert(buffer != nullptr);
        assert((buffer->desc.flags & RenderBufferFlag::FORMATTED) && "Buffer must allow formatted views.");

        this->buffer = buffer;
        this->format = format;
    }

    D3D12BufferFormattedView::~D3D12BufferFormattedView() { }

    // D3D12TextureView

    D3D12TextureView::D3D12TextureView(D3D12Texture *texture, const RenderTextureViewDesc &desc) {
        assert(texture != nullptr);

        this->texture = texture;
        this->format = toDXGI(desc.format);
        this->dimension = desc.dimension;
        this->mipLevels = desc.mipLevels;
        this->mipSlice = desc.mipSlice;
        this->shader4ComponentMapping = toD3D12(desc.componentMapping);
        
        // D3D12 and Vulkan disagree on whether D32 is usable as a texture view format. We just make D3D12 use R32 instead.
        if (format == DXGI_FORMAT_D32_FLOAT) {
            format = DXGI_FORMAT_R32_FLOAT;
        }
    }

    D3D12TextureView::~D3D12TextureView() { }

    // D3D12Texture

    D3D12Texture::D3D12Texture(D3D12Device *device, D3D12Pool *pool, const RenderTextureDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->pool = pool;
        this->desc = desc;

        const bool renderTarget = (desc.flags & RenderTextureFlag::RENDER_TARGET);
        const bool depthTarget = (desc.flags & RenderTextureFlag::DEPTH_TARGET);
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = toD3D12(desc.dimension);
        resourceDesc.Width = desc.width;
        resourceDesc.Height = desc.height;
        resourceDesc.DepthOrArraySize = desc.dimension == RenderTextureDimension::TEXTURE_3D ? desc.depth : desc.arraySize;
        resourceDesc.MipLevels = desc.mipLevels;
        resourceDesc.Format = toDXGI(desc.format);
        resourceDesc.SampleDesc.Count = desc.multisampling.sampleCount;
        resourceDesc.Layout = toD3D12(desc.textureArrangement);
        resourceDesc.Flags |= renderTarget ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= depthTarget ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL : D3D12_RESOURCE_FLAG_NONE;
        resourceDesc.Flags |= (desc.flags & RenderTextureFlag::UNORDERED_ACCESS) ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

        D3D12MA::ALLOCATION_DESC allocationDesc = {};
        allocationDesc.Flags = desc.committed ? D3D12MA::ALLOCATION_FLAG_COMMITTED : D3D12MA::ALLOCATION_FLAG_NONE;
        allocationDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        allocationDesc.CustomPool = (pool != nullptr) ? pool->d3d : nullptr;

        D3D12_CLEAR_VALUE optimizedClearValue;
        if (desc.optimizedClearValue != nullptr) {
            optimizedClearValue.Format = toDXGI(desc.optimizedClearValue->format);
            memcpy(optimizedClearValue.Color, desc.optimizedClearValue->color.rgba, sizeof(optimizedClearValue.Color));
        }

        HRESULT res = device->allocator->CreateResource(&allocationDesc, &resourceDesc, resourceStates, (desc.optimizedClearValue != nullptr) ? &optimizedClearValue : nullptr, &allocation, IID_PPV_ARGS(&d3d));
        if (FAILED(res)) {
            fprintf(stderr, "CreateResource failed with error code 0x%lX.\n", res);
            return;
        }

        if (renderTarget) {
            createRenderTargetHeap();
        }
        else if (depthTarget) {
            createDepthStencilHeap();
        }
    }

    D3D12Texture::~D3D12Texture() {
        releaseTargetHeap();

        if (allocation != nullptr) {
            d3d->Release();
            allocation->Release();
        }
    }

    std::unique_ptr<RenderTextureView> D3D12Texture::createTextureView(const RenderTextureViewDesc &desc) {
        return std::make_unique<D3D12TextureView>(this, desc);
    }

    void D3D12Texture::setName(const std::string &name) {
        setObjectName(d3d, name);
    }

    void D3D12Texture::createRenderTargetHeap() {
        targetAllocatorOffset = device->colorTargetHeapAllocator->allocate(1);
        if (targetAllocatorOffset == D3D12DescriptorHeapAllocator::INVALID_OFFSET) {
            fprintf(stderr, "Allocator was unable to find free space for the set.");
            return;
        }

        targetEntryCount = 1;
        targetHeapDepth = false;

        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = toDXGI(desc.format);

        const bool isMSAA = (desc.multisampling.sampleCount > RenderSampleCount::COUNT_1);
        switch (desc.dimension) {
        case RenderTextureDimension::TEXTURE_1D:
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
            rtvDesc.Texture1D.MipSlice = 0;
            break;
        case RenderTextureDimension::TEXTURE_2D:
            if (isMSAA) {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
            }
            else {
                rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                rtvDesc.Texture2D.PlaneSlice = 0;
            }

            break;
        case RenderTextureDimension::TEXTURE_3D:
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = 0;
            rtvDesc.Texture3D.FirstWSlice = 0;
            rtvDesc.Texture3D.WSize = 1;
            break;
        default:
            assert(false && "Unsupported texture dimension for render target.");
            break;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = device->colorTargetHeapAllocator->getCPUHandleAt(targetAllocatorOffset);
        device->d3d->CreateRenderTargetView(d3d, &rtvDesc, cpuHandle);
    }

    void D3D12Texture::createDepthStencilHeap() {
        targetAllocatorOffset = device->depthTargetHeapAllocator->allocate(2);
        if (targetAllocatorOffset == D3D12DescriptorHeapAllocator::INVALID_OFFSET) {
            fprintf(stderr, "Allocator was unable to find free space for the set.");
            return;
        }

        targetEntryCount = 2;
        targetHeapDepth = true;

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = toDXGI(desc.format);

        const bool isMSAA = (desc.multisampling.sampleCount > RenderSampleCount::COUNT_1);
        switch (desc.dimension) {
        case RenderTextureDimension::TEXTURE_1D:
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
            dsvDesc.Texture1D.MipSlice = 0;
            break;
        case RenderTextureDimension::TEXTURE_2D:
            if (isMSAA) {
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
            }
            else {
                dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice = 0;
            }

            break;
        default:
            assert(false && "Unsupported texture dimension for depth target.");
            break;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE writeHandle = device->depthTargetHeapAllocator->getCPUHandleAt(targetAllocatorOffset);
        const D3D12_CPU_DESCRIPTOR_HANDLE readOnlyHandle = device->depthTargetHeapAllocator->getCPUHandleAt(targetAllocatorOffset + 1);
        device->d3d->CreateDepthStencilView(d3d, &dsvDesc, writeHandle);

        dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        device->d3d->CreateDepthStencilView(d3d, &dsvDesc, readOnlyHandle);
    }

    void D3D12Texture::releaseTargetHeap() {
        if (targetEntryCount > 0) {
            if (targetHeapDepth) {
                device->depthTargetHeapAllocator->free(targetAllocatorOffset, targetEntryCount);
            }
            else {
                device->colorTargetHeapAllocator->free(targetAllocatorOffset, targetEntryCount);
            }

            targetEntryCount = 0;
        }
    }

    // D3D12AccelerationStructure

    D3D12AccelerationStructure::D3D12AccelerationStructure(D3D12Device *device, const RenderAccelerationStructureDesc &desc) {
        assert(device != nullptr);
        assert(desc.buffer.ref != nullptr);

        this->device = device;
        this->buffer = static_cast<const D3D12Buffer *>(desc.buffer.ref);
        this->offset = desc.buffer.offset;
        this->size = desc.size;
        this->type = desc.type;

        assert((buffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE) && "Buffer must be enabled for acceleration structures.");
    }

    D3D12AccelerationStructure::~D3D12AccelerationStructure() { }

    // D3D12Pool

    D3D12Pool::D3D12Pool(D3D12Device *device, const RenderPoolDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->desc = desc;

        D3D12MA::POOL_DESC poolDesc = {};
        poolDesc.HeapProperties.Type = toD3D12(desc.heapType);
        poolDesc.MinBlockCount = desc.minBlockCount;
        poolDesc.MaxBlockCount = desc.maxBlockCount;
        poolDesc.Flags |= desc.useLinearAlgorithm ? D3D12MA::POOL_FLAG_ALGORITHM_LINEAR : D3D12MA::POOL_FLAG_NONE;
        poolDesc.HeapFlags |= desc.allowOnlyBuffers ? D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS : D3D12_HEAP_FLAG_NONE;

        HRESULT res = device->allocator->CreatePool(&poolDesc, &d3d);
        if (FAILED(res)) {
            fprintf(stderr, "CreatePool failed with error code 0x%lX.\n", res);
            return;
        }
    }

    D3D12Pool::~D3D12Pool() {
        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    std::unique_ptr<RenderBuffer> D3D12Pool::createBuffer(const RenderBufferDesc &desc) {
        return std::make_unique<D3D12Buffer>(device, this, desc);
    }

    std::unique_ptr<RenderTexture> D3D12Pool::createTexture(const RenderTextureDesc &desc) {
        return std::make_unique<D3D12Texture>(device, this, desc);
    }

    // D3D12Shader

    D3D12Shader::D3D12Shader(D3D12Device *device, const void *data, uint64_t size, const char *entryPointName, RenderShaderFormat format) {
        assert(device != nullptr);
        assert(data != nullptr);
        assert(size > 0);
        assert(format != RenderShaderFormat::UNKNOWN);
        assert(format == RenderShaderFormat::DXIL);

        this->data = data;
        this->size = size;
        this->device = device;
        this->format = format;
        this->entryPointName = (entryPointName != nullptr) ? std::string(entryPointName) : std::string();
    }

    D3D12Shader::~D3D12Shader() { }

    // D3D12Sampler

    D3D12Sampler::D3D12Sampler(D3D12Device *device, const RenderSamplerDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->borderColor = desc.borderColor;
        this->shaderVisibility = desc.shaderVisibility;

        samplerDesc.Filter = toFilter(desc.minFilter, desc.magFilter, desc.mipmapMode, desc.anisotropyEnabled, desc.comparisonEnabled);
        samplerDesc.AddressU = toD3D12(desc.addressU);
        samplerDesc.AddressV = toD3D12(desc.addressV);
        samplerDesc.AddressW = toD3D12(desc.addressW);
        samplerDesc.MipLODBias = desc.mipLODBias;
        samplerDesc.MaxAnisotropy = desc.anisotropyEnabled ? desc.maxAnisotropy : 1;
        samplerDesc.ComparisonFunc = desc.comparisonEnabled ? toD3D12(desc.comparisonFunc) : D3D12_COMPARISON_FUNC_NEVER;
        samplerDesc.MinLOD = desc.minLOD;
        samplerDesc.MaxLOD = desc.maxLOD;

        float *dstColor = samplerDesc.BorderColor;
        switch (desc.borderColor) {
        case RenderBorderColor::TRANSPARENT_BLACK:
            dstColor[0] = 0.0f;
            dstColor[1] = 0.0f;
            dstColor[2] = 0.0f;
            dstColor[3] = 0.0f;
            break;
        case RenderBorderColor::OPAQUE_BLACK:
            dstColor[0] = 0.0f;
            dstColor[1] = 0.0f;
            dstColor[2] = 0.0f;
            dstColor[3] = 1.0f;
            break;
        case RenderBorderColor::OPAQUE_WHITE:
            dstColor[0] = 1.0f;
            dstColor[1] = 1.0f;
            dstColor[2] = 1.0f;
            dstColor[3] = 1.0f;
            break;
        default:
            assert(false && "Unknown border color.");
            break;
        }
    }

    D3D12Sampler::~D3D12Sampler() { }

    // D3D12Pipeline

    D3D12Pipeline::D3D12Pipeline(D3D12Device *device, Type type) {
        assert(device != nullptr);

        this->device = device;
        this->type = type;
    }

    D3D12Pipeline::~D3D12Pipeline() { }

    // D3D12ComputePipeline

    D3D12ComputePipeline::D3D12ComputePipeline(D3D12Device *device, const RenderComputePipelineDesc &desc) : D3D12Pipeline(device, Type::Compute) {
        assert(desc.pipelineLayout != nullptr);

        const D3D12PipelineLayout *rootSignature = static_cast<const D3D12PipelineLayout *>(desc.pipelineLayout);
        const D3D12Shader *computeShader = static_cast<const D3D12Shader *>(desc.computeShader);
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = rootSignature->rootSignature;
        psoDesc.CS.pShaderBytecode = computeShader->data;
        psoDesc.CS.BytecodeLength = computeShader->size;
        device->d3d->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&d3d));
    }

    D3D12ComputePipeline::~D3D12ComputePipeline() {
        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    void D3D12ComputePipeline::setName(const std::string& name) const {
        setObjectName(d3d, name);
    }

    RenderPipelineProgram D3D12ComputePipeline::getProgram(const std::string &name) const {
        assert(false && "Compute pipelines can't retrieve shader programs.");
        return RenderPipelineProgram();
    }

    // D3D12GraphicsPipeline

    D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12Device *device, const RenderGraphicsPipelineDesc &desc) : D3D12Pipeline(device, Type::Graphics) {
        assert(desc.pipelineLayout != nullptr);

        topology = toD3D12(desc.primitiveTopology);

        const D3D12PipelineLayout *pipelineLayout = static_cast<const D3D12PipelineLayout *>(desc.pipelineLayout);
        const D3D12Shader *vertexShader = static_cast<const D3D12Shader *>(desc.vertexShader);
        const D3D12Shader *geometryShader = static_cast<const D3D12Shader *>(desc.geometryShader);
        const D3D12Shader *pixelShader = static_cast<const D3D12Shader *>(desc.pixelShader);
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = pipelineLayout->rootSignature;
        psoDesc.VS.pShaderBytecode = (vertexShader != nullptr) ? vertexShader->data : nullptr;
        psoDesc.VS.BytecodeLength = (vertexShader != nullptr) ? vertexShader->size : 0;
        psoDesc.GS.pShaderBytecode = (geometryShader != nullptr) ? geometryShader->data : nullptr;
        psoDesc.GS.BytecodeLength = (geometryShader != nullptr) ? geometryShader->size : 0;
        psoDesc.PS.pShaderBytecode = (pixelShader != nullptr) ? pixelShader->data : nullptr;
        psoDesc.PS.BytecodeLength = (pixelShader != nullptr) ? pixelShader->size : 0;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.SampleDesc.Count = desc.multisampling.sampleCount;
        if (desc.primitiveTopology == RenderPrimitiveTopology::LINE_STRIP || desc.primitiveTopology == RenderPrimitiveTopology::TRIANGLE_STRIP) {
            psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
        }
        psoDesc.PrimitiveTopologyType = toTopologyType(desc.primitiveTopology);
        psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        psoDesc.RasterizerState.DepthClipEnable = desc.depthClipEnabled;
        psoDesc.RasterizerState.DepthBias = desc.depthBias;
        psoDesc.RasterizerState.SlopeScaledDepthBias = desc.slopeScaledDepthBias;

        switch (desc.cullMode) {
        case RenderCullMode::NONE:
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
            break;
        case RenderCullMode::FRONT:
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
            break;
        case RenderCullMode::BACK:
            psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
            break;
        default:
            assert(false && "Unknown cull mode.");
            return;
        }

        psoDesc.DepthStencilState.DepthEnable = desc.depthEnabled;
        psoDesc.DepthStencilState.DepthWriteMask = desc.depthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        psoDesc.DepthStencilState.DepthFunc = toD3D12(desc.depthFunction);
        psoDesc.NumRenderTargets = desc.renderTargetCount;
        psoDesc.BlendState.AlphaToCoverageEnable = desc.alphaToCoverageEnabled;

        for (uint32_t i = 0; i < desc.renderTargetCount; i++) {
            psoDesc.RTVFormats[i] = toDXGI(desc.renderTargetFormat[i]);
            
            const RenderBlendDesc &renderDesc = desc.renderTargetBlend[i];
            D3D12_RENDER_TARGET_BLEND_DESC &targetDesc = psoDesc.BlendState.RenderTarget[i];
            targetDesc.BlendEnable = renderDesc.blendEnabled;
            targetDesc.LogicOpEnable = desc.logicOpEnabled;
            targetDesc.SrcBlend = toD3D12(renderDesc.srcBlend);
            targetDesc.DestBlend = toD3D12(renderDesc.dstBlend);
            targetDesc.BlendOp = toD3D12(renderDesc.blendOp);
            targetDesc.SrcBlendAlpha = toD3D12(renderDesc.srcBlendAlpha);
            targetDesc.DestBlendAlpha = toD3D12(renderDesc.dstBlendAlpha);
            targetDesc.BlendOpAlpha = toD3D12(renderDesc.blendOpAlpha);
            targetDesc.LogicOp = toD3D12(desc.logicOp);
            targetDesc.RenderTargetWriteMask = renderDesc.renderTargetWriteMask;
        }

        psoDesc.DSVFormat = toDXGI(desc.depthTargetFormat);

        std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
        for (uint32_t i = 0; i < desc.inputElementsCount; i++) {
            const RenderInputElement &renderElement = desc.inputElements[i];
            D3D12_INPUT_ELEMENT_DESC inputElement;
            inputElement.SemanticName = renderElement.semanticName;
            inputElement.SemanticIndex = renderElement.semanticIndex;
            inputElement.Format = toDXGI(renderElement.format);
            inputElement.InputSlot = renderElement.slotIndex;
            inputElement.AlignedByteOffset = renderElement.alignedByteOffset;

            // Read the corresponding input slot to find the input classification and instance data step rate.
            bool foundInputSlot = false;
            D3D12_INPUT_CLASSIFICATION inputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            uint32_t instanceDataStepRate = 0;
            for (uint32_t j = 0; j < desc.inputSlotsCount; j++) {
                if (renderElement.slotIndex == desc.inputSlots[j].index) {
                    inputSlotClass = toD3D12(desc.inputSlots[j].classification);
                    instanceDataStepRate = (inputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA) ? 1 : 0;
                    foundInputSlot = true;
                    break;
                }
            }

            assert(foundInputSlot && "The slot index specified in the input element must exist in the input slots.");
            inputElement.InputSlotClass = inputSlotClass;
            inputElement.InstanceDataStepRate = instanceDataStepRate;
            inputElements.emplace_back(inputElement);
        }

        for (uint32_t i = 0; i < desc.inputSlotsCount; i++) {
            inputSlots.emplace_back(desc.inputSlots[i]);
        }

        psoDesc.InputLayout = { inputElements.data(), UINT(inputElements.size()) };

        device->d3d->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&d3d));
    }

    D3D12GraphicsPipeline::~D3D12GraphicsPipeline() {
        if (d3d != nullptr) {
            d3d->Release();
        }
    }

    void D3D12GraphicsPipeline::setName(const std::string& name) const {
        setObjectName(d3d, name);
    }

    RenderPipelineProgram D3D12GraphicsPipeline::getProgram(const std::string &name) const {
        assert(false && "Graphics pipelines can't retrieve shader programs.");
        return RenderPipelineProgram();
    }

    // D3D12RaytracingPipeline

    D3D12RaytracingPipeline::D3D12RaytracingPipeline(D3D12Device *device, const RenderRaytracingPipelineDesc &desc, const RenderPipeline *previousPipeline) : D3D12Pipeline(device, Type::Raytracing) {
        assert(desc.librariesCount > 0);
        assert(desc.pipelineLayout != nullptr);

        uint32_t subobjectCount = desc.librariesCount + desc.hitGroupsCount + 8;
        uint32_t exportSymbolsCount = 0;
        for (uint32_t i = 0; i < desc.librariesCount; i++) {
            exportSymbolsCount += desc.libraries[i].symbolsCount;
        }

        assert((exportSymbolsCount > 0) && "At least one symbol must be exported from the libraries.");

        std::vector<D3D12_STATE_SUBOBJECT> subobjects(subobjectCount);
        std::vector<D3D12_DXIL_LIBRARY_DESC> libraryDescs(desc.librariesCount);
        std::vector<D3D12_HIT_GROUP_DESC> hitGroupDescs(desc.hitGroupsCount);
        std::vector<D3D12_EXPORT_DESC> exportDescs(exportSymbolsCount);
        std::vector<std::wstring> exportNames(exportSymbolsCount);
        std::vector<std::wstring> exportRenames(exportSymbolsCount);
        std::unordered_set<std::wstring> exportAssociationSet;
        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION subobjectToExportsAssociation;

        uint32_t subobjectIndex = 0;
        uint32_t exportsIndex = 0;
        for (uint32_t i = 0; i < desc.librariesCount; i++) {
            uint32_t exportsIndexStart = exportsIndex;
            const RenderRaytracingPipelineLibrary &renderLibrary = desc.libraries[i];
            for (uint32_t j = 0; j < renderLibrary.symbolsCount; j++) {
                D3D12_EXPORT_DESC &exportDesc = exportDescs[exportsIndex];
                std::wstring &exportName = exportNames[exportsIndex];
                std::wstring &exportRename = exportRenames[exportsIndex];
                exportName = Utf8ToUtf16(std::string(renderLibrary.symbols[j].importName));
                exportRename = (renderLibrary.symbols[j].exportName != nullptr) ? Utf8ToUtf16(std::string(renderLibrary.symbols[j].exportName)) : exportName;
                exportDesc.Name = exportName.c_str();
                exportDesc.ExportToRename = exportRename.c_str();
                exportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
                exportAssociationSet.insert(exportRename);
                exportsIndex++;
            }

            const D3D12Shader *libraryShader = static_cast<const D3D12Shader *>(renderLibrary.shader);
            assert(libraryShader != nullptr);

            D3D12_DXIL_LIBRARY_DESC &libraryDesc = libraryDescs[i];
            libraryDesc.DXILLibrary.pShaderBytecode = libraryShader->data;
            libraryDesc.DXILLibrary.BytecodeLength = libraryShader->size;
            libraryDesc.pExports = &exportDescs[exportsIndexStart];
            libraryDesc.NumExports = exportsIndex - exportsIndexStart;

            D3D12_STATE_SUBOBJECT &subobject = subobjects[subobjectIndex++];
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
            subobject.pDesc = &libraryDescs[i];
        }

        auto fillHitGroupString = [&exportAssociationSet](LPCWSTR &dstPtr, std::wstring &dstString, const char *srcString, bool associateToSet) {
            if (srcString != nullptr) {
                dstString = Utf8ToUtf16(srcString);
                dstPtr = dstString.c_str();
            }
            else {
                dstPtr = nullptr;
            }

            if (associateToSet) {
                exportAssociationSet.insert(dstString);
            }
            else {
                exportAssociationSet.erase(dstString);
            }
        };

        std::vector<std::wstring> hitGroupNames(desc.hitGroupsCount * 4);
        uint32_t hitGroupNameIndex = 0;
        for (uint32_t i = 0; i < desc.hitGroupsCount; i++) {
            const RenderRaytracingPipelineHitGroup &renderHitGroup = desc.hitGroups[i];
            assert(renderHitGroup.hitGroupName != nullptr);

            D3D12_HIT_GROUP_DESC &hitGroupDesc = hitGroupDescs[i];
            hitGroupDesc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
            fillHitGroupString(hitGroupDesc.HitGroupExport, hitGroupNames[hitGroupNameIndex++], renderHitGroup.hitGroupName, true);
            fillHitGroupString(hitGroupDesc.ClosestHitShaderImport, hitGroupNames[hitGroupNameIndex++], renderHitGroup.closestHitName, false);
            fillHitGroupString(hitGroupDesc.AnyHitShaderImport, hitGroupNames[hitGroupNameIndex++], renderHitGroup.anyHitName, false);
            fillHitGroupString(hitGroupDesc.IntersectionShaderImport, hitGroupNames[hitGroupNameIndex++], renderHitGroup.intersectionName, false);

            D3D12_STATE_SUBOBJECT &subobject = subobjects[subobjectIndex++];
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
            subobject.pDesc = &hitGroupDescs[i];
        }

        D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
        shaderDesc.MaxPayloadSizeInBytes = desc.maxPayloadSize;
        shaderDesc.MaxAttributeSizeInBytes = desc.maxAttributeSize;

        D3D12_STATE_SUBOBJECT &shaderConfigSubobject = subobjects[subobjectIndex++];
        shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        shaderConfigSubobject.pDesc = &shaderDesc;

        std::vector<LPCWSTR> exportPointers;
        exportPointers.reserve(exportAssociationSet.size());
        for (const std::wstring &exportAssociation : exportAssociationSet) {
            exportPointers.emplace_back(exportAssociation.c_str());
        }

        D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION exportsAssociation = {};
        exportsAssociation.pExports = exportPointers.data();
        exportsAssociation.NumExports = static_cast<UINT>(exportPointers.size());
        exportsAssociation.pSubobjectToAssociate = &shaderConfigSubobject;

        D3D12_STATE_SUBOBJECT &exportAssociationSubobject = subobjects[subobjectIndex++];
        exportAssociationSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        exportAssociationSubobject.pDesc = &exportsAssociation;

        std::vector<std::wstring> associatedSymbolsNames(exportSymbolsCount);
        std::vector<LPCWSTR> associatedSymbolsNamesPointers(exportSymbolsCount);
        uint32_t associatedSymbolCount = 0;
        for (uint32_t i = 0; i < desc.librariesCount; i++) {
            const RenderRaytracingPipelineLibrary &renderLibrary = desc.libraries[i];
            for (uint32_t j = 0; j < renderLibrary.symbolsCount; j++) {
                const char *symbolName = (renderLibrary.symbols[j].exportName != nullptr) ? renderLibrary.symbols[j].exportName : renderLibrary.symbols[j].importName;
                associatedSymbolsNames[associatedSymbolCount] = Utf8ToUtf16(symbolName);
                associatedSymbolsNamesPointers[associatedSymbolCount] = associatedSymbolsNames[associatedSymbolCount].c_str();
                associatedSymbolCount++;
            }
        }

        pipelineLayout = static_cast<const D3D12PipelineLayout *>(desc.pipelineLayout);

        D3D12_STATE_SUBOBJECT &rootSignatureSubobject = subobjects[subobjectIndex++];
        rootSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        rootSignatureSubobject.pDesc = &pipelineLayout->rootSignature;

        subobjectToExportsAssociation.pExports = associatedSymbolsNamesPointers.data();
        subobjectToExportsAssociation.NumExports = associatedSymbolCount;
        subobjectToExportsAssociation.pSubobjectToAssociate = &rootSignatureSubobject;

        D3D12_STATE_SUBOBJECT &rootSignatureAssocationSubobject = subobjects[subobjectIndex++];
        rootSignatureAssocationSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        rootSignatureAssocationSubobject.pDesc = &subobjectToExportsAssociation;

        const D3D12PipelineLayout *interfaceGlobalPipelineLayout = static_cast<const D3D12PipelineLayout *>(device->rtDummyGlobalPipelineLayout.get());
        D3D12_STATE_SUBOBJECT &globalRootSignatureSubobject = subobjects[subobjectIndex++];
        globalRootSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
        globalRootSignatureSubobject.pDesc = &interfaceGlobalPipelineLayout->rootSignature;

        const D3D12PipelineLayout *interfaceLocalPipelineLayout = static_cast<const D3D12PipelineLayout *>(device->rtDummyLocalPipelineLayout.get());
        D3D12_STATE_SUBOBJECT &localRootSignatureSubobject = subobjects[subobjectIndex++];
        localRootSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        localRootSignatureSubobject.pDesc = &interfaceLocalPipelineLayout->rootSignature;

        D3D12_STATE_OBJECT_CONFIG stateConfig;
        stateConfig.Flags = desc.stateUpdateEnabled ? D3D12_STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS : D3D12_STATE_OBJECT_FLAG_NONE;

        D3D12_STATE_SUBOBJECT &stateConfigSubobject = subobjects[subobjectIndex++];
        stateConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_STATE_OBJECT_CONFIG;
        stateConfigSubobject.pDesc = &stateConfig;

        D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
        pipelineConfig.MaxTraceRecursionDepth = desc.maxRecursionDepth;

        D3D12_STATE_SUBOBJECT &pipelineConfigSubobject = subobjects[subobjectIndex++];
        pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
        pipelineConfigSubobject.pDesc = &pipelineConfig;

        assert(uint32_t(subobjects.size()) == subobjectIndex);

        D3D12_STATE_OBJECT_DESC pipelineDesc = {};
        pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        pipelineDesc.pSubobjects = subobjects.data();
        pipelineDesc.NumSubobjects = UINT(subobjects.size());

        if (desc.stateUpdateEnabled && (previousPipeline != nullptr)) {
            assert(static_cast<const D3D12Pipeline *>(previousPipeline)->type == Type::Raytracing);
            const D3D12RaytracingPipeline *previousRaytracingPipeline = static_cast<const D3D12RaytracingPipeline *>(previousPipeline);
            HRESULT res = device->d3d->AddToStateObject(&pipelineDesc, previousRaytracingPipeline->stateObject, IID_PPV_ARGS(&stateObject));
            if (FAILED(res)) {
                fprintf(stderr, "AddToStateObject failed with error code 0x%lX.\n", res);
                return;
            }
        }
        else {
            HRESULT res = device->d3d->CreateStateObject(&pipelineDesc, IID_PPV_ARGS(&stateObject));
            if (FAILED(res)) {
                fprintf(stderr, "CreateStateObject failed with error code 0x%lX.\n", res);
                return;
            }
        }

        HRESULT res = stateObject->QueryInterface(IID_PPV_ARGS(&stateObjectProperties));
        if (FAILED(res)) {
            fprintf(stderr, "QueryInterface failed with error code 0x%lX.\n", res);
            return;
        }

        // Cache all the programs compiled into the PSO into a name map.
        programShaderIdentifiers.reserve(exportAssociationSet.size());
        for (const std::wstring &exportAssociation : exportAssociationSet) {
            void *shaderIdentifier = stateObjectProperties->GetShaderIdentifier(exportAssociation.c_str());
            const std::string exportName = Utf16ToUtf8(exportAssociation);
            uint32_t programIndex = uint32_t(programShaderIdentifiers.size());
            programShaderIdentifiers.emplace_back(shaderIdentifier);
            nameProgramMap[exportName] = { programIndex };
        }
    }

    D3D12RaytracingPipeline::~D3D12RaytracingPipeline() {
        if (stateObjectProperties != nullptr) {
            stateObjectProperties->Release();
        }

        if (stateObject != nullptr) {
            stateObject->Release();
        }
    }

    void D3D12RaytracingPipeline::setName(const std::string& name) const {
        setObjectName(stateObject, name);
    }

    RenderPipelineProgram D3D12RaytracingPipeline::getProgram(const std::string &name) const {
        auto it = nameProgramMap.find(name);
        assert((it != nameProgramMap.end()) && "Program must exist in the PSO.");
        return it->second;
    }

    // D3D12PipelineLayout

    D3D12PipelineLayout::D3D12PipelineLayout(D3D12Device *device, const RenderPipelineLayoutDesc &desc) {
        assert(device != nullptr);

        this->device = device;
        this->setCount = desc.descriptorSetDescsCount;

        thread_local std::vector<D3D12_ROOT_PARAMETER> rootParameters;
        thread_local std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers;
        rootParameters.clear();
        staticSamplers.clear();

        // Push constants will be the first root parameters of the signature.
        for (uint32_t i = 0; i < desc.pushConstantRangesCount; i++) {
            const RenderPushConstantRange &range = desc.pushConstantRanges[i];
            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rootParameter.Constants.ShaderRegister = range.binding;
            rootParameter.Constants.RegisterSpace = range.set;
            rootParameter.Constants.Num32BitValues = (range.size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
            rootParameters.emplace_back(rootParameter);
            pushConstantRanges.emplace_back(range);
        }

        // Figure out the total size of ranges that will be needed first.
        uint32_t viewRangesCount = 0;
        uint32_t samplerRangesCount = 0;
        for (uint32_t i = 0; i < desc.descriptorSetDescsCount; i++) {
            const RenderDescriptorSetDesc &descriptorSetDesc = desc.descriptorSetDescs[i];
            for (uint32_t j = 0; j < descriptorSetDesc.descriptorRangesCount; j++) {
                const RenderDescriptorRange &renderRange = descriptorSetDesc.descriptorRanges[j];
                if (renderRange.immutableSampler != nullptr) {
                    continue;
                }
                else if (renderRange.type == RenderDescriptorRangeType::SAMPLER) {
                    samplerRangesCount++;
                }
                else {
                    viewRangesCount++;
                }
            }
        }

        thread_local std::vector<D3D12_DESCRIPTOR_RANGE> viewRanges;
        thread_local std::vector<D3D12_DESCRIPTOR_RANGE> samplerRanges;
        uint32_t viewRangeIndex = 0;
        uint32_t samplerRangeIndex = 0;
        viewRanges.resize(viewRangesCount);
        samplerRanges.resize(samplerRangesCount);

        // Descriptor sets will be created as descriptor table parameters.
        for (uint32_t i = 0; i < desc.descriptorSetDescsCount; i++) {
            uint32_t viewTableOffset = 0;
            uint32_t viewTableSize = 0;
            uint32_t samplerTableOffset = 0;
            uint32_t samplerTableSize = 0;
            const RenderDescriptorSetDesc &descriptorSetDesc = desc.descriptorSetDescs[i];
            for (uint32_t j = 0; j < descriptorSetDesc.descriptorRangesCount; j++) {
                // D3D12 requires specifying boundless arrays by setting the descriptor count to UINT_MAX.
                const RenderDescriptorRange &renderRange = descriptorSetDesc.descriptorRanges[j];
                const bool isRangeBoundless = (descriptorSetDesc.lastRangeIsBoundless && (j == (descriptorSetDesc.descriptorRangesCount - 1)));

                // Immutable samplers are converted to static samplers and filtered out of the table entirely.
                if (renderRange.immutableSampler != nullptr) {
                    for (uint32_t k = 0; k < renderRange.count; k++) {
                        const D3D12Sampler *sampler = static_cast<const D3D12Sampler *>(renderRange.immutableSampler[k]);
                        const D3D12_SAMPLER_DESC &samplerDesc = sampler->samplerDesc;
                        D3D12_STATIC_SAMPLER_DESC staticSampler = {};
                        staticSampler.Filter = samplerDesc.Filter;
                        staticSampler.AddressU = samplerDesc.AddressU;
                        staticSampler.AddressV = samplerDesc.AddressV;
                        staticSampler.AddressW = samplerDesc.AddressW;
                        staticSampler.MipLODBias = samplerDesc.MipLODBias;
                        staticSampler.MaxAnisotropy = samplerDesc.MaxAnisotropy;
                        staticSampler.ComparisonFunc = samplerDesc.ComparisonFunc;
                        staticSampler.BorderColor = toStaticBorderColor(sampler->borderColor);
                        staticSampler.MinLOD = samplerDesc.MinLOD;
                        staticSampler.MaxLOD = samplerDesc.MaxLOD;
                        staticSampler.ShaderRegister = renderRange.binding;
                        staticSampler.RegisterSpace = i;
                        staticSampler.ShaderVisibility = toD3D12(sampler->shaderVisibility);
                        staticSamplers.emplace_back(staticSampler);
                    }
                }
                // Dynamic samplers must use a different type of heap.
                else if (renderRange.type == RenderDescriptorRangeType::SAMPLER) {
                    D3D12_DESCRIPTOR_RANGE &descriptorRange = samplerRanges[samplerRangeIndex + samplerTableSize];
                    descriptorRange.RangeType = toRangeType(renderRange.type);
                    descriptorRange.NumDescriptors = isRangeBoundless ? UINT_MAX : renderRange.count;
                    descriptorRange.BaseShaderRegister = renderRange.binding;
                    descriptorRange.RegisterSpace = i;
                    descriptorRange.OffsetInDescriptorsFromTableStart = samplerTableOffset;
                    samplerTableSize++;
                    samplerTableOffset += renderRange.count;
                }
                else {
                    D3D12_DESCRIPTOR_RANGE &descriptorRange = viewRanges[viewRangeIndex + viewTableSize];
                    descriptorRange.RangeType = toRangeType(renderRange.type);
                    descriptorRange.NumDescriptors = isRangeBoundless ? UINT_MAX : renderRange.count;
                    descriptorRange.BaseShaderRegister = renderRange.binding;
                    descriptorRange.RegisterSpace = i;
                    descriptorRange.OffsetInDescriptorsFromTableStart = viewTableOffset;
                    viewTableSize++;
                    viewTableOffset += renderRange.count;
                }
            }

            setViewRootIndices.emplace_back(uint32_t(rootParameters.size()));

            if (viewTableSize > 0) {
                D3D12_ROOT_PARAMETER rootParameter = {};
                rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParameter.DescriptorTable.pDescriptorRanges = &viewRanges[viewRangeIndex];
                rootParameter.DescriptorTable.NumDescriptorRanges = viewTableSize;
                rootParameters.emplace_back(rootParameter);
                viewRangeIndex += viewTableSize;
            }

            setSamplerRootIndices.emplace_back(uint32_t(rootParameters.size()));

            if (samplerTableSize > 0) {
                D3D12_ROOT_PARAMETER rootParameter = {};
                rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                rootParameter.DescriptorTable.pDescriptorRanges = &samplerRanges[samplerRangeIndex];
                rootParameter.DescriptorTable.NumDescriptorRanges = samplerTableSize;
                rootParameters.emplace_back(rootParameter);
                samplerRangeIndex += samplerTableSize;
            }
        }

        // Add root descriptors last.
        for (uint32_t i = 0; i < desc.rootDescriptorDescsCount; i++) {
            const RenderRootDescriptorDesc& rootDescriptorDesc = desc.rootDescriptorDescs[i];

            D3D12_ROOT_PARAMETER rootParameter = {};            
            rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            rootParameter.Descriptor.ShaderRegister = rootDescriptorDesc.shaderRegister;
            rootParameter.Descriptor.RegisterSpace = rootDescriptorDesc.registerSpace;

            switch (rootDescriptorDesc.type) {
            case RenderRootDescriptorType::CONSTANT_BUFFER:
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                break;
            case RenderRootDescriptorType::SHADER_RESOURCE:
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
                break;
            case RenderRootDescriptorType::UNORDERED_ACCESS:
                rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
                break;
            default:
                assert(false && "Invalid root descriptor type.");
                break;
            }

            rootDescriptorRootIndicesAndTypes.emplace_back(uint32_t(rootParameters.size()), rootDescriptorDesc.type);
            rootParameters.push_back(rootParameter);
        }

        // Store the total amount of root parameters.
        rootCount = rootParameters.size();

        // Fill root signature desc.
        D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
        rootSignatureDesc.Flags |= desc.isLocal ? D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE : D3D12_ROOT_SIGNATURE_FLAG_NONE;
        rootSignatureDesc.Flags |= desc.allowInputLayout ? D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT : D3D12_ROOT_SIGNATURE_FLAG_NONE;
        rootSignatureDesc.pParameters = !rootParameters.empty() ? rootParameters.data() : nullptr;
        rootSignatureDesc.NumParameters = UINT(rootParameters.size());
        rootSignatureDesc.pStaticSamplers = !staticSamplers.empty() ? staticSamplers.data() : nullptr;
        rootSignatureDesc.NumStaticSamplers = UINT(staticSamplers.size());

        // Serialize the root signature.
        ID3DBlob *signatureBlob;
        ID3DBlob *errorBlob;
        HRESULT res = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlob);
        if (FAILED(res)) {
            fprintf(stderr, "%s\n", (char *)(errorBlob->GetBufferPointer()));
            return;
        }

        res = device->d3d->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        if (FAILED(res)) {
            fprintf(stderr, "CreateRootSignature failed with error code 0x%lX.\n", res);
            return;
        }
    }

    D3D12PipelineLayout::~D3D12PipelineLayout() {
        if (rootSignature != nullptr) {
            rootSignature->Release();
        }
    }

    // D3D12Device

    D3D12Device::D3D12Device(D3D12Interface *renderInterface) {
        assert(renderInterface != nullptr);

        this->renderInterface = renderInterface;
        
        // Detect adapter to use that will offer the best performance and features.
        HRESULT res;
        UINT adapterIndex = 0;
        IDXGIAdapter1 *adapterOption = nullptr;
        while (renderInterface->dxgiFactory->EnumAdapters1(adapterIndex++, &adapterOption) != DXGI_ERROR_NOT_FOUND) {
            DXGI_ADAPTER_DESC1 adapterDesc;
            adapterOption->GetDesc1(&adapterDesc);

            // Ignore remote or software adapters.
            if (adapterDesc.Flags & (DXGI_ADAPTER_FLAG_REMOTE | DXGI_ADAPTER_FLAG_SOFTWARE)) {
                adapterOption->Release();
                continue;
            }

            ID3D12Device8 *deviceOption = nullptr;
            res = D3D12CreateDevice(adapterOption, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&deviceOption));
            if (FAILED(res)) {
                adapterOption->Release();
                continue;
            }

            // Determine the shader model supported by the device.
#       if SM_5_1_SUPPORTED
            const D3D_SHADER_MODEL supportedShaderModels[] = { D3D_SHADER_MODEL_6_0, D3D_SHADER_MODEL_5_1 };
#       else
            const D3D_SHADER_MODEL supportedShaderModels[] = { D3D_SHADER_MODEL_6_0 };
#       endif
            D3D12_FEATURE_DATA_SHADER_MODEL dataShaderModel = {};
            for (uint32_t i = 0; i < _countof(supportedShaderModels); i++) {
                dataShaderModel.HighestShaderModel = supportedShaderModels[i];
                res = deviceOption->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &dataShaderModel, sizeof(dataShaderModel));
                if (res != E_INVALIDARG) {
                    if (FAILED(res)) {
                        deviceOption->Release();
                        adapterOption->Release();
                        continue;
                    }

                    break;
                }
            }

            // Determine if the device supports sample locations.
            bool samplePositionsOption = false;
            D3D12_FEATURE_DATA_D3D12_OPTIONS2 d3d12Options2 = {};
            res = deviceOption->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &d3d12Options2, sizeof(d3d12Options2));
            if (SUCCEEDED(res)) {
                samplePositionsOption = d3d12Options2.ProgrammableSamplePositionsTier >= D3D12_PROGRAMMABLE_SAMPLE_POSITIONS_TIER_1;
            }

            // Determine if the device supports raytracing.
            bool rtSupportOption = false;
            bool rtStateUpdateSupportOption = false;
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 d3d12Options5 = {};
            res = deviceOption->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &d3d12Options5, sizeof(d3d12Options5));
            if (SUCCEEDED(res)) {
                rtSupportOption = d3d12Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;
                rtStateUpdateSupportOption = d3d12Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1;
            }

            // Check if triangle fan is supported.
            bool triangleFanSupportOption = false;
            D3D12_FEATURE_DATA_D3D12_OPTIONS15 d3d12Options15 = {};
            res = deviceOption->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS15, &d3d12Options15, sizeof(d3d12Options15));
            if (SUCCEEDED(res)) {
                triangleFanSupportOption = d3d12Options15.TriangleFanSupported;
            }

            // Pick this adapter and device if it has better feature support than the current one.
            bool preferOverNothing = (adapter == nullptr) || (d3d == nullptr);
            bool preferVideoMemory = adapterDesc.DedicatedVideoMemory > description.dedicatedVideoMemory;
            bool preferUserChoice = false;//wcsstr(adapterDesc.Description, L"AMD") != nullptr;
            bool preferOption = preferOverNothing || preferVideoMemory || preferUserChoice;
            if (preferOption) {
                if (d3d != nullptr) {
                    d3d->Release();
                }

                if (adapter != nullptr) {
                    adapter->Release();
                }

                adapter = adapterOption;
                d3d = deviceOption;
                shaderModel = dataShaderModel.HighestShaderModel;
                capabilities.raytracing = rtSupportOption;
                capabilities.raytracingStateUpdate = rtStateUpdateSupportOption;
                capabilities.sampleLocations = samplePositionsOption;
                capabilities.triangleFan = triangleFanSupportOption;
                description.name = Utf16ToUtf8(adapterDesc.Description);
                description.dedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;

                if (preferUserChoice) {
                    break;
                }
            }
            else {
                deviceOption->Release();
                adapterOption->Release();
            }
        }

        if (d3d == nullptr) {
            fprintf(stderr, "Unable to create a D3D12 device with the required features.\n");
            return;
        }

        D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.pDevice = d3d;
        allocatorDesc.pAdapter = adapter;
        allocatorDesc.Flags = D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED | 
            D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED | D3D12MA::ALLOCATOR_FLAG_DONT_PREFER_SMALL_BUFFERS_COMMITTED;

        res = D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
        if (FAILED(res)) {
            fprintf(stderr, "D3D12MA::CreateAllocator failed with error code 0x%lX.\n", res);
            release();
            return;
        }

        if (capabilities.raytracing) {
            RenderPipelineLayoutDesc pipelineLayoutDesc;
            rtDummyGlobalPipelineLayout = createPipelineLayout(pipelineLayoutDesc);

            pipelineLayoutDesc.isLocal = true;
            rtDummyLocalPipelineLayout = createPipelineLayout(pipelineLayoutDesc);
        }

#   ifdef D3D12_DEBUG_LAYER_ENABLED
        // Add it to the debug layer info queue if available.
        ID3D12InfoQueue *infoQueue;
        res = d3d->QueryInterface(IID_PPV_ARGS(&infoQueue));
        if (SUCCEEDED(res)) {
            D3D12_MESSAGE_SEVERITY severities[] = {
                D3D12_MESSAGE_SEVERITY_INFO
            };

            D3D12_MESSAGE_ID denyIds[] = {
                D3D12_MESSAGE_ID_COMMAND_LIST_DRAW_VERTEX_BUFFER_NOT_SET,
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_DRAW_EMPTY_SCISSOR_RECTANGLE,
                D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,
                D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_RENDERTARGETVIEW_NOT_SET,
#           ifdef D3D12_DEBUG_LAYER_SUPRESS_SAMPLE_POSITIONS_ERROR
                D3D12_MESSAGE_ID_SAMPLEPOSITIONS_MISMATCH_RECORDTIME_ASSUMEDFROMCLEAR,
                D3D12_MESSAGE_ID_SAMPLEPOSITIONS_MISMATCH_DEFERRED,
#           endif
            };
            
            D3D12_INFO_QUEUE_FILTER newFilter = {};
            newFilter.DenyList.NumSeverities = _countof(severities);
            newFilter.DenyList.pSeverityList = severities;
            newFilter.DenyList.NumIDs = _countof(denyIds);
            newFilter.DenyList.pIDList = denyIds;
            infoQueue->PushStorageFilter(&newFilter);

            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, D3D12_DEBUG_LAYER_BREAK_ON_ERROR);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, D3D12_DEBUG_LAYER_BREAK_ON_WARNING);
        }
#   endif
        
        // Fill capabilities.
        capabilities.descriptorIndexing = true;
        capabilities.scalarBlockLayout = true;
        capabilities.presentWait = true;
        capabilities.preferHDR = description.dedicatedVideoMemory > (512 * 1024 * 1024);

        // Create descriptor heaps allocator.
        viewHeapAllocator = std::make_unique<D3D12DescriptorHeapAllocator>(this, ShaderDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        samplerHeapAllocator = std::make_unique<D3D12DescriptorHeapAllocator>(this, SamplerDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        colorTargetHeapAllocator = std::make_unique<D3D12DescriptorHeapAllocator>(this, TargetDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        depthTargetHeapAllocator = std::make_unique<D3D12DescriptorHeapAllocator>(this, TargetDescriptorHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    D3D12Device::~D3D12Device() {
        viewHeapAllocator.reset();
        samplerHeapAllocator.reset();
        rtDummyGlobalPipelineLayout.reset();
        rtDummyLocalPipelineLayout.reset();
        release();
    }

    std::unique_ptr<RenderCommandList> D3D12Device::createCommandList(RenderCommandListType type) {
        return std::make_unique<D3D12CommandList>(this, type);
    }

    std::unique_ptr<RenderDescriptorSet> D3D12Device::createDescriptorSet(const RenderDescriptorSetDesc &desc) {
        return std::make_unique<D3D12DescriptorSet>(this, desc);
    }

    std::unique_ptr<RenderShader> D3D12Device::createShader(const void *data, uint64_t size, const char *entryPointName, RenderShaderFormat format) {
        return std::make_unique<D3D12Shader>(this, data, size, entryPointName, format);
    }

    std::unique_ptr<RenderSampler> D3D12Device::createSampler(const RenderSamplerDesc &desc) {
        return std::make_unique<D3D12Sampler>(this, desc);
    }

    std::unique_ptr<RenderPipeline> D3D12Device::createComputePipeline(const RenderComputePipelineDesc &desc) {
        return std::make_unique<D3D12ComputePipeline>(this, desc);
    }

    std::unique_ptr<RenderPipeline> D3D12Device::createGraphicsPipeline(const RenderGraphicsPipelineDesc &desc) {
        return std::make_unique<D3D12GraphicsPipeline>(this, desc);
    }

    std::unique_ptr<RenderPipeline> D3D12Device::createRaytracingPipeline(const RenderRaytracingPipelineDesc &desc, const RenderPipeline *previousPipeline) {
        return std::make_unique<D3D12RaytracingPipeline>(this, desc, previousPipeline);
    }

    std::unique_ptr<RenderCommandQueue> D3D12Device::createCommandQueue(RenderCommandListType type) {
        return std::make_unique<D3D12CommandQueue>(this, type);
    }
    
    std::unique_ptr<RenderBuffer> D3D12Device::createBuffer(const RenderBufferDesc &desc) {
        return std::make_unique<D3D12Buffer>(this, nullptr, desc);
    }

    std::unique_ptr<RenderTexture> D3D12Device::createTexture(const RenderTextureDesc &desc) {
        return std::make_unique<D3D12Texture>(this, nullptr, desc);
    }

    std::unique_ptr<RenderAccelerationStructure> D3D12Device::createAccelerationStructure(const RenderAccelerationStructureDesc &desc) {
        return std::make_unique<D3D12AccelerationStructure>(this, desc);
    }

    std::unique_ptr<RenderPool> D3D12Device::createPool(const RenderPoolDesc &desc) {
        return std::make_unique<D3D12Pool>(this, desc);
    }

    std::unique_ptr<RenderPipelineLayout> D3D12Device::createPipelineLayout(const RenderPipelineLayoutDesc &desc) {
        return std::make_unique<D3D12PipelineLayout>(this, desc);
    }

    std::unique_ptr<RenderCommandFence> D3D12Device::createCommandFence() {
        return std::make_unique<D3D12CommandFence>(this);
    }

    std::unique_ptr<RenderCommandSemaphore> D3D12Device::createCommandSemaphore() {
        return std::make_unique<D3D12CommandSemaphore>(this);
    }

    std::unique_ptr<RenderFramebuffer> D3D12Device::createFramebuffer(const RenderFramebufferDesc &desc) {
        return std::make_unique<D3D12Framebuffer>(this, desc);
    }

    void D3D12Device::setBottomLevelASBuildInfo(RenderBottomLevelASBuildInfo &buildInfo, const RenderBottomLevelASMesh *meshes, uint32_t meshCount, bool preferFastBuild, bool preferFastTrace) {
        assert(meshes != nullptr);
        assert(meshCount > 0);

        buildInfo.buildData.resize(sizeof(D3D12_RAYTRACING_GEOMETRY_DESC) * meshCount, 0);

        D3D12_RAYTRACING_GEOMETRY_DESC *geometryDescs = reinterpret_cast<D3D12_RAYTRACING_GEOMETRY_DESC *>(buildInfo.buildData.data());
        for (uint32_t i = 0; i < meshCount; i++) {
            const RenderBottomLevelASMesh &mesh = meshes[i];
            const D3D12Buffer *interfaceIndexBuffer = static_cast<const D3D12Buffer *>(mesh.indexBuffer.ref);
            const D3D12Buffer *interfaceVertexBuffer = static_cast<const D3D12Buffer *>(mesh.vertexBuffer.ref);
            assert((interfaceIndexBuffer == nullptr) || ((interfaceIndexBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input must be allowed on index buffer."));
            assert((interfaceVertexBuffer == nullptr) || ((interfaceVertexBuffer->desc.flags & RenderBufferFlag::ACCELERATION_STRUCTURE_INPUT) && "Acceleration structure input must be allowed on vertex buffer."));

            D3D12_RAYTRACING_GEOMETRY_DESC &geometryDesc = geometryDescs[i];
            geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
            geometryDesc.Flags |= mesh.isOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
            geometryDesc.Triangles.Transform3x4 = 0;
            geometryDesc.Triangles.IndexFormat = toDXGI(mesh.indexFormat);
            geometryDesc.Triangles.VertexFormat = toDXGI(mesh.vertexFormat);
            geometryDesc.Triangles.IndexCount = mesh.indexCount;
            geometryDesc.Triangles.VertexCount = mesh.vertexCount;
            geometryDesc.Triangles.IndexBuffer = (interfaceIndexBuffer != nullptr) ? (interfaceIndexBuffer->d3d->GetGPUVirtualAddress() + mesh.indexBuffer.offset) : 0;
            geometryDesc.Triangles.VertexBuffer.StartAddress = (interfaceVertexBuffer != nullptr) ? (interfaceVertexBuffer->d3d->GetGPUVirtualAddress() + mesh.vertexBuffer.offset) : 0;
            geometryDesc.Triangles.VertexBuffer.StrideInBytes = mesh.vertexStride;
        }

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.NumDescs = meshCount;
        inputs.Flags = toRTASBuildFlags(preferFastBuild, preferFastTrace);
        inputs.pGeometryDescs = geometryDescs;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
        d3d->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

        buildInfo.meshCount = meshCount;
        buildInfo.preferFastBuild = preferFastBuild;
        buildInfo.preferFastTrace = preferFastTrace;
        buildInfo.scratchSize = roundUp(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        buildInfo.accelerationStructureSize = roundUp(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    void D3D12Device::setTopLevelASBuildInfo(RenderTopLevelASBuildInfo &buildInfo, const RenderTopLevelASInstance *instances, uint32_t instanceCount, bool preferFastBuild, bool preferFastTrace) {
        assert(instances != nullptr);
        assert(instanceCount > 0);

        uint64_t bufferSize = roundUp(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instanceCount, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        buildInfo.instancesBufferData.resize(bufferSize, 0);

        D3D12_RAYTRACING_INSTANCE_DESC *instanceDescs = reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC *>(buildInfo.instancesBufferData.data());
        for (uint32_t i = 0; i < instanceCount; i++) {
            const RenderTopLevelASInstance &instance = instances[i];
            const D3D12Buffer *interfaceBottomLevelAS = static_cast<const D3D12Buffer *>(instance.bottomLevelAS.ref);
            assert(interfaceBottomLevelAS != nullptr);

            D3D12_RAYTRACING_INSTANCE_DESC &instanceDesc = instanceDescs[i];
            instanceDesc.InstanceID = instance.instanceID;
            instanceDesc.InstanceMask = instance.instanceMask;
            instanceDesc.InstanceContributionToHitGroupIndex = instance.instanceContributionToHitGroupIndex;
            instanceDesc.Flags = instance.cullDisable ? D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE : D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
            instanceDesc.AccelerationStructure = interfaceBottomLevelAS->d3d->GetGPUVirtualAddress() + instance.bottomLevelAS.offset;
            memcpy(instanceDesc.Transform, instance.transform.m, sizeof(instanceDesc.Transform));
        }

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
        inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        inputs.Flags = toRTASBuildFlags(preferFastBuild, preferFastTrace);
        inputs.NumDescs = instanceCount;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
        d3d->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

        buildInfo.instanceCount = instanceCount;
        buildInfo.preferFastBuild = preferFastBuild;
        buildInfo.preferFastTrace = preferFastTrace;
        buildInfo.scratchSize = roundUp(info.ScratchDataSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
        buildInfo.accelerationStructureSize = roundUp(info.ResultDataMaxSizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    }

    void D3D12Device::setShaderBindingTableInfo(RenderShaderBindingTableInfo &tableInfo, const RenderShaderBindingGroups &groups, const RenderPipeline *pipeline, RenderDescriptorSet **descriptorSets, uint32_t descriptorSetCount) {
        assert(pipeline != nullptr);
        assert(descriptorSets != nullptr);

        const D3D12RaytracingPipeline *raytracingPipeline = static_cast<const D3D12RaytracingPipeline *>(pipeline);
        assert((raytracingPipeline->type == D3D12Pipeline::Type::Raytracing) && "Only raytracing pipelines can be used to build shader binding tables.");
        assert((raytracingPipeline->pipelineLayout->setCount <= descriptorSetCount) && "There must be enough descriptor sets available for the pipeline.");

        uint64_t tableSize = 0;
        auto setGroup = [&](RenderShaderBindingGroupInfo &groupInfo, const RenderShaderBindingGroup &renderGroup) {
            groupInfo.startIndex = 0;

            if (renderGroup.pipelineProgramsCount == 0) {
                groupInfo.stride = 0;
                groupInfo.offset = 0;
                groupInfo.size = 0;
            }
            else {
                groupInfo.stride = roundUp(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT + sizeof(UINT64) * raytracingPipeline->pipelineLayout->rootCount, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
                groupInfo.offset = tableSize;
                groupInfo.size = groupInfo.stride * renderGroup.pipelineProgramsCount;
                tableSize += groupInfo.size;
            }
        };

        setGroup(tableInfo.groups.rayGen, groups.rayGen);
        setGroup(tableInfo.groups.miss, groups.miss);
        setGroup(tableInfo.groups.hitGroup, groups.hitGroup);
        setGroup(tableInfo.groups.callable, groups.callable);

        tableSize = roundUp(tableSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

        tableInfo.tableBufferData.clear();
        tableInfo.tableBufferData.resize(tableSize, 0);
        
        thread_local std::vector<UINT64> descriptorHandles;
        descriptorHandles.clear();
        descriptorHandles.resize(raytracingPipeline->pipelineLayout->rootCount, 0);

        for (uint32_t i = 0; i < raytracingPipeline->pipelineLayout->setCount; i++) {
            const D3D12DescriptorSet *interfaceDescriptorSet = static_cast<const D3D12DescriptorSet *>(descriptorSets[i]);
            if (interfaceDescriptorSet != nullptr) {
                if (interfaceDescriptorSet->viewAllocation.count > 0) {
                    uint32_t viewRootIndex = raytracingPipeline->pipelineLayout->setViewRootIndices[i];
                    descriptorHandles[viewRootIndex] = viewHeapAllocator->getGPUHandleAt(interfaceDescriptorSet->viewAllocation.offset).ptr;
                }

                if (interfaceDescriptorSet->samplerAllocation.count > 0) {
                    uint32_t samplerRootIndex = raytracingPipeline->pipelineLayout->setSamplerRootIndices[i];
                    descriptorHandles[samplerRootIndex] = samplerHeapAllocator->getGPUHandleAt(interfaceDescriptorSet->samplerAllocation.offset).ptr;
                }
            }
        }

        auto copyGroupData = [&](RenderShaderBindingGroupInfo &groupInfo, const RenderShaderBindingGroup &renderGroup) {
            for (uint32_t i = 0; i < renderGroup.pipelineProgramsCount; i++) {
                void *shaderId = raytracingPipeline->programShaderIdentifiers[renderGroup.pipelinePrograms[i].programIndex];
                uint64_t tableOffset = groupInfo.offset + i * groupInfo.stride;
                memcpy(&tableInfo.tableBufferData[tableOffset], shaderId, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

                if (raytracingPipeline->pipelineLayout->rootCount > 0) {
                    UINT64 *tableDescriptorHandles = reinterpret_cast<UINT64 *>(&tableInfo.tableBufferData[tableOffset + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT]);
                    memcpy(tableDescriptorHandles, descriptorHandles.data(), sizeof(UINT64) * raytracingPipeline->pipelineLayout->rootCount);
                }
            }
        };

        copyGroupData(tableInfo.groups.rayGen, groups.rayGen);
        copyGroupData(tableInfo.groups.miss, groups.miss);
        copyGroupData(tableInfo.groups.hitGroup, groups.hitGroup);
        copyGroupData(tableInfo.groups.callable, groups.callable);
    }

    const RenderDeviceCapabilities &D3D12Device::getCapabilities() const {
        return capabilities;
    }

    const RenderDeviceDescription &D3D12Device::getDescription() const {
        return description;
    }

    RenderSampleCounts D3D12Device::getSampleCountsSupported(RenderFormat format) const {
        HRESULT res;
        RenderSampleCounts countsSupported = RenderSampleCount::COUNT_0;
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS multisampleLevel = {};
        RenderSampleCounts testCount = RenderSampleCount::COUNT_1;
        while (testCount <= RenderSampleCount::COUNT_MAX) {
            multisampleLevel.SampleCount = testCount;
            multisampleLevel.Format = toDXGI(format);

            res = d3d->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &multisampleLevel, sizeof(multisampleLevel));
            if (SUCCEEDED(res)) {
                if (multisampleLevel.NumQualityLevels > 0) {
                    countsSupported |= testCount;
                }
            }

            testCount = testCount << 1;
        }

        return countsSupported;
    }

    void D3D12Device::waitIdle() const {
        assert(false && "Use fences to replicate wait idle behavior on D3D12.");
    }

    void D3D12Device::release() {
        if (d3d != nullptr) {
            d3d->Release();
            d3d = nullptr;
        }

        if (adapter != nullptr) {
            adapter->Release();
            adapter = nullptr;
        }
    }

    bool D3D12Device::isValid() const {
        return d3d != nullptr;
    }

    // D3D12Interface

    D3D12Interface::D3D12Interface() {
        // Create DXGI Factory.
        UINT dxgiFactoryFlags = 0;

#   ifdef D3D12_DEBUG_LAYER_ENABLED
        ID3D12Debug1 *debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();
#       ifdef D3D12_DEBUG_LAYER_GPU_BASED_VALIDATION_ENABLED
            debugController->SetEnableGPUBasedValidation(TRUE);
#       endif

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#   endif

        HRESULT res = CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory));
        if (FAILED(res)) {
            fprintf(stderr, "CreateDXGIFactory2 failed with error code 0x%lX.\n", res);
            return;
        }

        // Fill capabilities.
        capabilities.shaderFormat = RenderShaderFormat::DXIL;
    }

    D3D12Interface::~D3D12Interface() {
        if (dxgiFactory != nullptr) {
            dxgiFactory->Release();
        }
    }

    std::unique_ptr<RenderDevice> D3D12Interface::createDevice() {
        std::unique_ptr<D3D12Device> createdDevice = std::make_unique<D3D12Device>(this);
        return createdDevice->isValid() ? std::move(createdDevice) : nullptr;
    }

    const RenderInterfaceCapabilities &D3D12Interface::getCapabilities() const {
        return capabilities;
    }

    bool D3D12Interface::isValid() const {
        return dxgiFactory != nullptr;
    }

    // Global creation function.
    
    std::unique_ptr<RenderInterface> CreateD3D12Interface() {
        std::unique_ptr<D3D12Interface> createdInterface = std::make_unique<D3D12Interface>();
        return createdInterface->isValid() ? std::move(createdInterface) : nullptr;
    }
};
