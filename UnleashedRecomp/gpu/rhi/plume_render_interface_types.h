//
// plume
//
// Copyright (c) 2024 renderbag and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file for details.
//

#pragma once

#include <cassert>
#include <list>
#include <memory>
#include <string>
#include <vector>
#include <cfloat>

#if defined(_WIN64)
#include <Windows.h>
#elif defined(__ANDROID__)
#include "android/native_window.h"
#elif defined(__linux__)
#include "X11/Xlib.h"
#undef None
#undef Status
#undef LockMask
#undef ControlMask
#undef Success
#elif defined(__APPLE__)
typedef struct _NSWindow NSWindow;
#endif

#ifdef SDL_VULKAN_ENABLED
#include <SDL_vulkan.h>
#endif

namespace plume {
#if defined(_WIN64)
    // Native HWND handle to the target window.
    typedef HWND RenderWindow;
#elif defined(__ANDROID__)
    typedef ANativeWindow* RenderWindow;
#elif defined(SDL_VULKAN_ENABLED)
    typedef SDL_Window *RenderWindow;
#elif defined(__linux__)
    struct RenderWindow {
        Display* display;
        Window window;
        bool operator==(const struct RenderWindow& rhs) const {
            return display == rhs.display && window == rhs.window;
        }
        bool operator!=(const struct RenderWindow& rhs) const { return !(*this == rhs); }
    };
#elif defined(__APPLE__)
    struct RenderWindow {
        NSWindow* window;
        bool operator==(const struct RenderWindow& rhs) const {
            return window == rhs.window;
        }
        bool operator!=(const struct RenderWindow& rhs) const { return !(*this == rhs); }
    };
#else
    static_assert(false, "RenderWindow was not defined for this platform.");
#endif

    struct RenderBuffer;
    struct RenderDescriptorSet;
    struct RenderPipeline;
    struct RenderPipelineLayout;
    struct RenderSampler;
    struct RenderShader;
    struct RenderTexture;

    // Enums.

    enum class RenderFormat {
        UNKNOWN,
        R32G32B32A32_TYPELESS,
        R32G32B32A32_FLOAT,
        R32G32B32A32_UINT,
        R32G32B32A32_SINT,
        R32G32B32_TYPELESS,
        R32G32B32_FLOAT,
        R32G32B32_UINT,
        R32G32B32_SINT,
        R16G16B16A16_TYPELESS,
        R16G16B16A16_FLOAT,
        R16G16B16A16_UNORM,
        R16G16B16A16_UINT,
        R16G16B16A16_SNORM,
        R16G16B16A16_SINT,
        R32G32_TYPELESS,
        R32G32_FLOAT,
        R32G32_UINT,
        R32G32_SINT,
        R8G8B8A8_TYPELESS,
        R8G8B8A8_UNORM,
        R8G8B8A8_UINT,
        R8G8B8A8_SNORM,
        R8G8B8A8_SINT,
        B8G8R8A8_UNORM,
        R16G16_TYPELESS,
        R16G16_FLOAT,
        R16G16_UNORM,
        R16G16_UINT,
        R16G16_SNORM,
        R16G16_SINT,
        R32_TYPELESS,
        D32_FLOAT,
        R32_FLOAT,
        R32_UINT,
        R32_SINT,
        R8G8_TYPELESS,
        R8G8_UNORM,
        R8G8_UINT,
        R8G8_SNORM,
        R8G8_SINT,
        R16_TYPELESS,
        R16_FLOAT,
        D16_UNORM,
        R16_UNORM,
        R16_UINT,
        R16_SNORM,
        R16_SINT,
        R8_TYPELESS,
        R8_UNORM,
        R8_UINT,
        R8_SNORM,
        R8_SINT,
        BC1_TYPELESS,
        BC1_UNORM,
        BC1_UNORM_SRGB,
        BC2_TYPELESS,
        BC2_UNORM,
        BC2_UNORM_SRGB,
        BC3_TYPELESS,
        BC3_UNORM,
        BC3_UNORM_SRGB,
        BC4_TYPELESS,
        BC4_UNORM,
        BC4_SNORM,
        BC5_TYPELESS,
        BC5_UNORM,
        BC5_SNORM,
        BC6H_TYPELESS,
        BC6H_UF16,
        BC6H_SF16,
        BC7_TYPELESS,
        BC7_UNORM,
        BC7_UNORM_SRGB
    };

    enum class RenderTextureDimension {
        UNKNOWN,
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D
    };

    enum class RenderTextureViewDimension {
        UNKNOWN,
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D,
        TEXTURE_CUBE
    };

    enum class RenderCommandListType {
        UNKNOWN,
        DIRECT,
        COMPUTE,
        COPY
    };

    enum class RenderPrimitiveTopology {
        UNKNOWN,
        POINT_LIST,
        LINE_LIST,
        LINE_STRIP,
        TRIANGLE_LIST,
        TRIANGLE_STRIP,
        TRIANGLE_FAN
    };

    enum class RenderSRVType {
        UNKNOWN,
        BUFFER,
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D
    };

    enum class RenderUAVType {
        UNKNOWN,
        BUFFER,
        TEXTURE_1D,
        TEXTURE_2D,
        TEXTURE_3D
    };

    enum class RenderCullMode {
        UNKNOWN,
        NONE,
        FRONT,
        BACK
    };

    enum class RenderComparisonFunction {
        UNKNOWN,
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS
    };

    enum class RenderInputSlotClassification {
        UNKNOWN,
        PER_VERTEX_DATA,
        PER_INSTANCE_DATA
    };

    enum class RenderBlend {
        UNKNOWN,
        ZERO,
        ONE,
        SRC_COLOR,
        INV_SRC_COLOR,
        SRC_ALPHA,
        INV_SRC_ALPHA,
        DEST_ALPHA,
        INV_DEST_ALPHA,
        DEST_COLOR,
        INV_DEST_COLOR,
        SRC_ALPHA_SAT,
        BLEND_FACTOR,
        INV_BLEND_FACTOR,
        SRC1_COLOR,
        INV_SRC1_COLOR,
        SRC1_ALPHA,
        INV_SRC1_ALPHA
    };

    enum class RenderBlendOperation {
        UNKNOWN,
        ADD,
        SUBTRACT,
        REV_SUBTRACT,
        MIN,
        MAX
    };

    enum class RenderColorWriteEnable : uint8_t {
        UNKNOWN = 0x0,
        RED = 0x1,
        GREEN = 0x2,
        BLUE = 0x4,
        ALPHA = 0x8,
        ALL = RED | GREEN | BLUE | ALPHA
    };

    enum class RenderLogicOperation {
        UNKNOWN,
        CLEAR,
        SET,
        COPY,
        COPY_INVERTED,
        NOOP,
        INVERT,
        AND,
        NAND,
        OR,
        NOR,
        XOR,
        EQUIV,
        AND_REVERSE,
        AND_INVERTED,
        OR_REVERSE,
        OR_INVERTED
    };

    enum class RenderFilter {
        UNKNOWN,
        NEAREST,
        LINEAR
    };

    enum class RenderMipmapMode {
        UNKNOWN,
        NEAREST,
        LINEAR
    };

    enum class RenderTextureAddressMode {
        UNKNOWN,
        WRAP,
        MIRROR,
        CLAMP,
        BORDER,
        MIRROR_ONCE
    };

    enum class RenderBorderColor {
        UNKNOWN,
        TRANSPARENT_BLACK,
        OPAQUE_BLACK,
        OPAQUE_WHITE
    };

    enum class RenderShaderVisibility {
        UNKNOWN,
        ALL,
        VERTEX,
        GEOMETRY,
        PIXEL
    };

    enum class RenderDescriptorRangeType {
        UNKNOWN,
        CONSTANT_BUFFER,
        FORMATTED_BUFFER,
        READ_WRITE_FORMATTED_BUFFER,
        TEXTURE,
        READ_WRITE_TEXTURE,
        SAMPLER,
        STRUCTURED_BUFFER,
        READ_WRITE_STRUCTURED_BUFFER,
        BYTE_ADDRESS_BUFFER,
        READ_WRITE_BYTE_ADDRESS_BUFFER,
        ACCELERATION_STRUCTURE
    };

    enum class RenderRootDescriptorType {
        UNKNOWN,
        CONSTANT_BUFFER,
        SHADER_RESOURCE,
        UNORDERED_ACCESS
    };

    enum class RenderHeapType {
        UNKNOWN,
        DEFAULT,
        UPLOAD,
        READBACK
    };

    enum class RenderTextureArrangement {
        UNKNOWN,
        ROW_MAJOR
    };

    enum class RenderShaderFormat {
        UNKNOWN,
        DXIL,
        SPIRV
    };

    enum class RenderRaytracingPipelineLibrarySymbolType {
        UNKNOWN,
        RAYGEN,
        MISS,
        CLOSEST_HIT,
        ANY_HIT,
        INTERSECTION,
        CALLABLE
    };

    enum class RenderAccelerationStructureType {
        UNKNOWN,
        TOP_LEVEL,
        BOTTOM_LEVEL
    };

    namespace RenderShaderStageFlag {
        enum Bits : uint32_t {
            NONE = 0U,
            VERTEX = 1U << 0,
            GEOMETRY = 1U << 1,
            PIXEL = 1U << 2,
            COMPUTE = 1U << 3,
            RAYGEN = 1U << 4,
            ANY_HIT = 1U << 5,
            CLOSEST_HIT = 1U << 6,
            MISS = 1U << 7,
            INTERSECTION = 1U << 8,
            CALLABLE = 1U << 9
        };
    };

    typedef uint32_t RenderShaderStageFlags;

    namespace RenderBufferFlag {
        enum Bits : uint32_t {
            NONE = 0U,
            VERTEX = 1U << 0,
            INDEX = 1U << 1,
            STORAGE = 1U << 2,
            CONSTANT = 1U << 3,
            FORMATTED = 1U << 4,
            ACCELERATION_STRUCTURE = 1U << 5,
            ACCELERATION_STRUCTURE_INPUT = 1U << 6,
            ACCELERATION_STRUCTURE_SCRATCH = 1U << 7,
            SHADER_BINDING_TABLE = 1U << 8,
            UNORDERED_ACCESS = 1U << 9
        };
    };

    typedef uint32_t RenderBufferFlags;

    namespace RenderTextureFlag {
        enum Bits : uint32_t {
            NONE = 0U,
            RENDER_TARGET = 1U << 0,
            DEPTH_TARGET = 1U << 1,
            STORAGE = 1U << 2,
            UNORDERED_ACCESS = 1U << 3,
            CUBE = 1U << 4
        };
    };

    typedef uint32_t RenderTextureFlags;

    namespace RenderBarrierStage {
        enum Bits : uint32_t {
            NONE = 0U,
            GRAPHICS = 1U << 0,
            COMPUTE = 1U << 1,
            COPY = 1U << 2,
            GRAPHICS_AND_COMPUTE = GRAPHICS | COMPUTE,
            ALL = GRAPHICS | COMPUTE | COPY
        };
    };

    typedef uint32_t RenderBarrierStages;

    namespace RenderBufferAccess {
        enum Bits : uint32_t {
            NONE = 0U,
            READ = 1U << 0,
            WRITE = 1U << 1
        };
    };

    typedef uint32_t RenderBufferAccessBits;

    enum class RenderTextureLayout {
        UNKNOWN,
        GENERAL,
        SHADER_READ,
        COLOR_WRITE,
        DEPTH_WRITE,
        DEPTH_READ,
        COPY_SOURCE,
        COPY_DEST,
        RESOLVE_SOURCE,
        RESOLVE_DEST,
        PRESENT
    };
    
    namespace RenderSampleCount {
        enum Bits : uint32_t {
            COUNT_0 = 0x0,
            COUNT_1 = 0x1,
            COUNT_2 = 0x2,
            COUNT_4 = 0x4,
            COUNT_8 = 0x8,
            COUNT_16 = 0x10,
            COUNT_32 = 0x20,
            COUNT_64 = 0x40,
            COUNT_MAX = COUNT_64
        };
    };

    typedef uint32_t RenderSampleCounts;

    enum class RenderDeviceType {
        UNKNOWN,
        INTEGRATED,
        DISCRETE,
        VIRTUAL,
        CPU
    };

    // Global functions.

    constexpr uint32_t RenderFormatSize(RenderFormat format) {
        switch (format) {
        case RenderFormat::R32G32B32A32_TYPELESS:
        case RenderFormat::R32G32B32A32_FLOAT:
        case RenderFormat::R32G32B32A32_UINT:
        case RenderFormat::R32G32B32A32_SINT:
            return 16;
        case RenderFormat::R32G32B32_TYPELESS:
        case RenderFormat::R32G32B32_FLOAT:
        case RenderFormat::R32G32B32_UINT:
        case RenderFormat::R32G32B32_SINT:
            return 12;
        case RenderFormat::R16G16B16A16_TYPELESS:
        case RenderFormat::R16G16B16A16_FLOAT:
        case RenderFormat::R16G16B16A16_UNORM:
        case RenderFormat::R16G16B16A16_UINT:
        case RenderFormat::R16G16B16A16_SNORM:
        case RenderFormat::R16G16B16A16_SINT:
        case RenderFormat::R32G32_TYPELESS:
        case RenderFormat::R32G32_FLOAT:
        case RenderFormat::R32G32_UINT:
        case RenderFormat::R32G32_SINT:
            return 8;
        case RenderFormat::R8G8B8A8_TYPELESS:
        case RenderFormat::R8G8B8A8_UNORM:
        case RenderFormat::R8G8B8A8_UINT:
        case RenderFormat::R8G8B8A8_SNORM:
        case RenderFormat::R8G8B8A8_SINT:
        case RenderFormat::B8G8R8A8_UNORM:
        case RenderFormat::R16G16_TYPELESS:
        case RenderFormat::R16G16_FLOAT:
        case RenderFormat::R16G16_UNORM:
        case RenderFormat::R16G16_UINT:
        case RenderFormat::R16G16_SNORM:
        case RenderFormat::R16G16_SINT:
        case RenderFormat::R32_TYPELESS:
        case RenderFormat::D32_FLOAT:
        case RenderFormat::R32_FLOAT:
        case RenderFormat::R32_UINT:
        case RenderFormat::R32_SINT:
            return 4;
        case RenderFormat::R8G8_TYPELESS:
        case RenderFormat::R8G8_UNORM:
        case RenderFormat::R8G8_UINT:
        case RenderFormat::R8G8_SNORM:
        case RenderFormat::R8G8_SINT:
        case RenderFormat::R16_TYPELESS:
        case RenderFormat::R16_FLOAT:
        case RenderFormat::D16_UNORM:
        case RenderFormat::R16_UNORM:
        case RenderFormat::R16_UINT:
        case RenderFormat::R16_SNORM:
        case RenderFormat::R16_SINT:
            return 2;
        case RenderFormat::R8_TYPELESS:
        case RenderFormat::R8_UNORM:
        case RenderFormat::R8_UINT:
        case RenderFormat::R8_SNORM:
        case RenderFormat::R8_SINT:
            return 1;
        case RenderFormat::BC1_UNORM:
        case RenderFormat::BC1_UNORM_SRGB:
        case RenderFormat::BC1_TYPELESS:
        case RenderFormat::BC4_UNORM:
        case RenderFormat::BC4_SNORM:
        case RenderFormat::BC4_TYPELESS:
            return 8;
        case RenderFormat::BC2_UNORM:
        case RenderFormat::BC2_UNORM_SRGB:
        case RenderFormat::BC2_TYPELESS:
        case RenderFormat::BC3_UNORM:
        case RenderFormat::BC3_UNORM_SRGB:
        case RenderFormat::BC3_TYPELESS:
        case RenderFormat::BC5_UNORM:
        case RenderFormat::BC5_SNORM:
        case RenderFormat::BC6H_UF16:
        case RenderFormat::BC6H_SF16:
        case RenderFormat::BC7_UNORM:
        case RenderFormat::BC7_UNORM_SRGB:
            return 16;
        default:
            assert(false && "Unknown format.");
            return 1;
        }
    }

    constexpr uint32_t RenderFormatBlockWidth(RenderFormat format) {
        switch (format) {
        case RenderFormat::R32G32B32A32_TYPELESS:
        case RenderFormat::R32G32B32A32_FLOAT:
        case RenderFormat::R32G32B32A32_UINT:
        case RenderFormat::R32G32B32A32_SINT:
        case RenderFormat::R32G32B32_TYPELESS:
        case RenderFormat::R32G32B32_FLOAT:
        case RenderFormat::R32G32B32_UINT:
        case RenderFormat::R32G32B32_SINT:
        case RenderFormat::R16G16B16A16_TYPELESS:
        case RenderFormat::R16G16B16A16_FLOAT:
        case RenderFormat::R16G16B16A16_UNORM:
        case RenderFormat::R16G16B16A16_UINT:
        case RenderFormat::R16G16B16A16_SNORM:
        case RenderFormat::R16G16B16A16_SINT:
        case RenderFormat::R32G32_TYPELESS:
        case RenderFormat::R32G32_FLOAT:
        case RenderFormat::R32G32_UINT:
        case RenderFormat::R32G32_SINT:
        case RenderFormat::R8G8B8A8_TYPELESS:
        case RenderFormat::R8G8B8A8_UNORM:
        case RenderFormat::R8G8B8A8_UINT:
        case RenderFormat::R8G8B8A8_SNORM:
        case RenderFormat::R8G8B8A8_SINT:
        case RenderFormat::B8G8R8A8_UNORM:
        case RenderFormat::R16G16_TYPELESS:
        case RenderFormat::R16G16_FLOAT:
        case RenderFormat::R16G16_UNORM:
        case RenderFormat::R16G16_UINT:
        case RenderFormat::R16G16_SNORM:
        case RenderFormat::R16G16_SINT:
        case RenderFormat::R32_TYPELESS:
        case RenderFormat::D32_FLOAT:
        case RenderFormat::R32_FLOAT:
        case RenderFormat::R32_UINT:
        case RenderFormat::R32_SINT:
        case RenderFormat::R8G8_TYPELESS:
        case RenderFormat::R8G8_UNORM:
        case RenderFormat::R8G8_UINT:
        case RenderFormat::R8G8_SNORM:
        case RenderFormat::R8G8_SINT:
        case RenderFormat::R16_TYPELESS:
        case RenderFormat::R16_FLOAT:
        case RenderFormat::D16_UNORM:
        case RenderFormat::R16_UNORM:
        case RenderFormat::R16_UINT:
        case RenderFormat::R16_SNORM:
        case RenderFormat::R16_SINT:
        case RenderFormat::R8_TYPELESS:
        case RenderFormat::R8_UNORM:
        case RenderFormat::R8_UINT:
        case RenderFormat::R8_SNORM:
        case RenderFormat::R8_SINT:
            return 1;
        case RenderFormat::BC1_TYPELESS:
        case RenderFormat::BC1_UNORM:
        case RenderFormat::BC1_UNORM_SRGB:
        case RenderFormat::BC2_TYPELESS:
        case RenderFormat::BC2_UNORM:
        case RenderFormat::BC2_UNORM_SRGB:
        case RenderFormat::BC3_TYPELESS:
        case RenderFormat::BC3_UNORM:
        case RenderFormat::BC3_UNORM_SRGB:
        case RenderFormat::BC4_TYPELESS:
        case RenderFormat::BC4_UNORM:
        case RenderFormat::BC4_SNORM:
        case RenderFormat::BC5_TYPELESS:
        case RenderFormat::BC5_UNORM:
        case RenderFormat::BC5_SNORM:
        case RenderFormat::BC6H_TYPELESS:
        case RenderFormat::BC6H_UF16:
        case RenderFormat::BC6H_SF16:
        case RenderFormat::BC7_TYPELESS:
        case RenderFormat::BC7_UNORM:
        case RenderFormat::BC7_UNORM_SRGB:
            return 4;
        default:
            assert(false && "Unknown format.");
            return 1;
        }
    };

    // Concrete structs.

    struct RenderColor {
        union {
            struct {
                float rgba[4];
            };

            struct {
                float r;
                float g;
                float b;
                float a;
            };
        };

        RenderColor() {
            r = 0.0f;
            g = 0.0f;
            b = 0.0f;
            a = 1.0f;
        }

        RenderColor(float r, float g, float b, float a = 1.0f) {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
    };

    struct RenderAffineTransform {
        float m[3][4] = {};

        RenderAffineTransform() {
            m[0][0] = 1.0f;
            m[1][1] = 1.0f;
            m[2][2] = 1.0f;
        }
    };

    struct RenderDepth {
        float depth = 1.0f;

        RenderDepth() = default;

        RenderDepth(float depth) {
            this->depth = depth;
        }
    };
    
    struct RenderMultisamplingLocation {
        // Valid range is [-8, 7].
        int8_t x = 0;
        int8_t y = 0;
    };

    struct RenderMultisampling {
        RenderSampleCounts sampleCount = RenderSampleCount::COUNT_1;
        RenderMultisamplingLocation sampleLocations[16] = {};
        bool sampleLocationsEnabled = false;

        RenderMultisampling() = default;

        RenderMultisampling(RenderSampleCounts sampleCount) {
            this->sampleCount = sampleCount;
        }
    };

    struct RenderBufferReference {
        const RenderBuffer *ref = nullptr;
        uint64_t offset = 0;

        RenderBufferReference() = default;

        RenderBufferReference(const RenderBuffer *ref) {
            this->ref = ref;
            offset = 0;
        }

        RenderBufferReference(const RenderBuffer *ref, uint64_t offset) {
            this->ref = ref;
            this->offset = offset;
        }

        bool operator==(const RenderBufferReference& rhs) const {
            return ref == rhs.ref && offset == rhs.offset;
        }

        bool operator!=(const RenderBufferReference& rhs) const {
            return !(*this == rhs);
        }
    };

    struct RenderBufferBarrier {
        RenderBuffer *buffer = nullptr;
        RenderBufferAccessBits accessBits = RenderBufferAccess::NONE;

        RenderBufferBarrier() = default;

        RenderBufferBarrier(RenderBuffer *buffer, RenderBufferAccessBits accessBits) {
            this->buffer = buffer;
            this->accessBits = accessBits;
        }
    };

    struct RenderBufferStructuredView {
        uint32_t structureByteStride = 0;
        uint32_t firstElement = 0;

        RenderBufferStructuredView() = default;

        RenderBufferStructuredView(uint32_t structureByteStride, uint32_t firstElement = 0) {
            this->structureByteStride = structureByteStride;
            this->firstElement = firstElement;
        }
    };

    struct RenderTextureBarrier {
        RenderTexture *texture = nullptr;
        RenderTextureLayout layout = RenderTextureLayout::UNKNOWN;

        RenderTextureBarrier() = default;

        RenderTextureBarrier(RenderTexture *texture, RenderTextureLayout layout) {
            this->texture = texture;
            this->layout = layout;
        }
    };

    struct RenderClearValue {
        RenderFormat format = RenderFormat::UNKNOWN;
        union {
            RenderColor color;
            RenderDepth depth;
        };

        RenderClearValue() : color{} {}

        static RenderClearValue Color(RenderColor color, RenderFormat format) {
            RenderClearValue clear = {};
            clear.format = format;
            clear.color = color;
            return clear;
        }

        static RenderClearValue Depth(RenderDepth depth, RenderFormat format) {
            RenderClearValue clear = {};
            clear.format = format;
            clear.depth = depth;
            return clear;
        }
    };

    struct RenderBufferDesc {
        uint64_t size = 0;
        RenderHeapType heapType = RenderHeapType::UNKNOWN;
        RenderBufferFlags flags = RenderBufferFlag::NONE;
        bool committed = false;

        RenderBufferDesc() = default;

        static RenderBufferDesc DefaultBuffer(uint64_t size, RenderBufferFlags flags = RenderBufferFlag::NONE) {
            RenderBufferDesc desc;
            desc.size = size;
            desc.heapType = RenderHeapType::DEFAULT;
            desc.flags = flags;
            return desc;
        }

        static RenderBufferDesc UploadBuffer(uint64_t size, RenderBufferFlags flags = RenderBufferFlag::NONE) {
            RenderBufferDesc desc;
            desc.heapType = RenderHeapType::UPLOAD;
            desc.size = size;
            desc.flags = flags;
            return desc;
        }

        static RenderBufferDesc ReadbackBuffer(uint64_t size, RenderBufferFlags flags = RenderBufferFlag::NONE) {
            RenderBufferDesc desc;
            desc.heapType = RenderHeapType::READBACK;
            desc.size = size;
            desc.flags = flags;
            return desc;
        }

        static RenderBufferDesc VertexBuffer(uint64_t size, RenderHeapType heapType, RenderBufferFlags flags = RenderBufferFlag::NONE) {
            RenderBufferDesc desc;
            desc.size = size;
            desc.heapType = heapType;
            desc.flags = flags | RenderBufferFlag::VERTEX;
            return desc;
        }

        static RenderBufferDesc IndexBuffer(uint64_t size, RenderHeapType heapType, RenderBufferFlags flags = RenderBufferFlag::NONE) {
            RenderBufferDesc desc;
            desc.size = size;
            desc.heapType = heapType;
            desc.flags = flags | RenderBufferFlag::INDEX;
            return desc;
        }

        static RenderBufferDesc AccelerationStructureBuffer(uint64_t size) {
            RenderBufferDesc desc;
            desc.size = size;
            desc.heapType = RenderHeapType::DEFAULT;
            desc.flags = RenderBufferFlag::ACCELERATION_STRUCTURE;
            return desc;
        }
    };

    struct RenderTextureDesc {
        RenderTextureDimension dimension = RenderTextureDimension::UNKNOWN;
        uint32_t width = 0;
        uint32_t height = 0;
        uint16_t depth = 0;
        uint16_t mipLevels = 0;
        uint16_t arraySize = 0;
        RenderMultisampling multisampling;
        RenderFormat format = RenderFormat::UNKNOWN;
        RenderTextureArrangement textureArrangement = RenderTextureArrangement::UNKNOWN;
        const RenderClearValue *optimizedClearValue = nullptr;
        RenderTextureFlags flags = RenderTextureFlag::NONE;
        bool committed = false;

        RenderTextureDesc() = default;

        static RenderTextureDesc Texture(RenderTextureDimension dimension, uint32_t width, uint32_t height, uint16_t depth, uint16_t mipLevels, uint16_t arraySize, RenderFormat format, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            RenderTextureDesc desc;
            desc.dimension = dimension;
            desc.width = width;
            desc.height = height;
            desc.depth = depth;
            desc.mipLevels = mipLevels;
            desc.arraySize = arraySize;
            desc.format = format;
            desc.flags = flags;
            return desc;
        }

        static RenderTextureDesc Texture1D(uint32_t width, uint16_t mipLevels, RenderFormat format, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            return Texture(RenderTextureDimension::TEXTURE_1D, width, 1, 1, mipLevels, 1, format, flags);
        }

        static RenderTextureDesc Texture2D(uint32_t width, uint32_t height, uint16_t mipLevels, RenderFormat format, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            return Texture(RenderTextureDimension::TEXTURE_2D, width, height, 1, mipLevels, 1, format, flags);
        }

        static RenderTextureDesc Texture3D(uint32_t width, uint32_t height, uint32_t depth, uint16_t mipLevels, RenderFormat format, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            return Texture(RenderTextureDimension::TEXTURE_3D, width, height, depth, mipLevels, 1, format, flags);
        }

        static RenderTextureDesc ColorTarget(uint32_t width, uint32_t height, RenderFormat format, RenderMultisampling multisampling = RenderMultisampling(), const RenderClearValue *optimizedClearValue = nullptr, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            RenderTextureDesc desc;
            desc.committed = true;
            desc.dimension = RenderTextureDimension::TEXTURE_2D;
            desc.width = width;
            desc.height = height;
            desc.depth = 1;
            desc.mipLevels = 1;
            desc.arraySize = 1;
            desc.format = format;
            desc.multisampling = multisampling;
            desc.flags = flags | RenderTextureFlag::RENDER_TARGET;
            desc.optimizedClearValue = optimizedClearValue;
            return desc;
        }

        static RenderTextureDesc DepthTarget(uint32_t width, uint32_t height, RenderFormat format, RenderMultisampling multisampling = RenderMultisampling(), const RenderClearValue *optimizedClearValue = nullptr, RenderTextureFlags flags = RenderTextureFlag::NONE) {
            RenderTextureDesc desc;
            desc.committed = true;
            desc.dimension = RenderTextureDimension::TEXTURE_2D;
            desc.width = width;
            desc.height = height;
            desc.depth = 1;
            desc.mipLevels = 1;
            desc.arraySize = 1;
            desc.format = format;
            desc.multisampling = multisampling;
            desc.flags = flags | RenderTextureFlag::DEPTH_TARGET;
            desc.optimizedClearValue = optimizedClearValue;
            return desc;
        }
    };

    enum class RenderSwizzle : uint8_t
    {
        IDENTITY = 0,
        ZERO = 1,
        ONE = 2,
        R = 3,
        G = 4,
        B = 5,
        A = 6
    };

    struct RenderComponentMapping
    {
        RenderSwizzle r = RenderSwizzle::IDENTITY;
        RenderSwizzle g = RenderSwizzle::IDENTITY;
        RenderSwizzle b = RenderSwizzle::IDENTITY;
        RenderSwizzle a = RenderSwizzle::IDENTITY;

        RenderComponentMapping() = default;
        RenderComponentMapping(RenderSwizzle r, RenderSwizzle g, RenderSwizzle b, RenderSwizzle a) : r(r), g(g), b(b), a(a) {}
    };

    struct RenderTextureViewDesc {
        RenderFormat format = RenderFormat::UNKNOWN;
        RenderTextureViewDimension dimension = RenderTextureViewDimension::UNKNOWN;
        uint32_t mipLevels = 0;
        uint32_t mipSlice = 0;
        RenderComponentMapping componentMapping;

        RenderTextureViewDesc() = default;

        static RenderTextureViewDesc Texture1D(RenderFormat format, uint32_t mipLevels = 1) {
            RenderTextureViewDesc viewDesc;
            viewDesc.format = format;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_1D;
            viewDesc.mipLevels = mipLevels;
            return viewDesc;
        }

        static RenderTextureViewDesc Texture2D(RenderFormat format, uint32_t mipLevels = 1) {
            RenderTextureViewDesc viewDesc;
            viewDesc.format = format;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_2D;
            viewDesc.mipLevels = mipLevels;
            return viewDesc;
        }

        static RenderTextureViewDesc Texture3D(RenderFormat format, uint32_t mipLevels = 1) {
            RenderTextureViewDesc viewDesc;
            viewDesc.format = format;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_3D;
            viewDesc.mipLevels = mipLevels;
            return viewDesc;
        }      
        
        static RenderTextureViewDesc TextureCube(RenderFormat format, uint32_t mipLevels = 1) {
            RenderTextureViewDesc viewDesc;
            viewDesc.format = format;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_CUBE;
            viewDesc.mipLevels = mipLevels;
            return viewDesc;
        }
    };

    struct RenderAccelerationStructureDesc {
        RenderAccelerationStructureType type = RenderAccelerationStructureType::UNKNOWN;
        RenderBufferReference buffer;
        uint64_t size = 0;

        RenderAccelerationStructureDesc() = default;

        RenderAccelerationStructureDesc(RenderAccelerationStructureType type, RenderBufferReference buffer, uint64_t size) {
            this->type = type;
            this->buffer = buffer;
            this->size = size;
        }
    };

    enum class RenderTextureCopyType {
        UNKNOWN,
        SUBRESOURCE,
        PLACED_FOOTPRINT
    };

    struct RenderTextureCopyLocation {
        const RenderTexture *texture = nullptr;
        const RenderBuffer *buffer = nullptr;
        RenderTextureCopyType type = RenderTextureCopyType::UNKNOWN;

        union {
            struct {
                RenderFormat format;
                uint32_t width;
                uint32_t height;
                uint32_t depth;
                uint32_t rowWidth;
                uint64_t offset;
            } placedFootprint;

            struct {
                uint32_t index;
            } subresource;
        };

        static RenderTextureCopyLocation PlacedFootprint(const RenderBuffer *buffer, RenderFormat format, uint32_t width, uint32_t height, uint32_t depth, uint32_t rowWidth, uint64_t offset = 0) {
            RenderTextureCopyLocation loc;
            loc.buffer = buffer;
            loc.type = RenderTextureCopyType::PLACED_FOOTPRINT;
            loc.placedFootprint.format = format;
            loc.placedFootprint.width = width;
            loc.placedFootprint.height = height;
            loc.placedFootprint.depth = depth;
            loc.placedFootprint.rowWidth = rowWidth;
            loc.placedFootprint.offset = offset;
            return loc;
        }

        static RenderTextureCopyLocation Subresource(const RenderTexture *texture, uint32_t index = 0) {
            RenderTextureCopyLocation loc;
            loc.texture = texture;
            loc.type = RenderTextureCopyType::SUBRESOURCE;
            loc.subresource.index = index;
            return loc;
        }
    };

    struct RenderPoolDesc {
        RenderHeapType heapType = RenderHeapType::UNKNOWN;
        uint32_t minBlockCount = 0;
        uint32_t maxBlockCount = 0;
        bool useLinearAlgorithm = false;
        bool allowOnlyBuffers = false;
    };

    struct RenderInputSlot {
        uint32_t index = 0;
        uint32_t stride = 0;
        RenderInputSlotClassification classification = RenderInputSlotClassification::UNKNOWN;

        RenderInputSlot() = default;

        RenderInputSlot(uint32_t index, uint32_t stride, RenderInputSlotClassification classification = RenderInputSlotClassification::PER_VERTEX_DATA) {
            this->index = index;
            this->stride = stride;
            this->classification = classification;
        }
    };

    struct RenderInputElement {
        // Semantic name and index and location must be specified for both backends, but each attribute will only be read by the backend that uses them.
        const char *semanticName = nullptr;
        uint32_t semanticIndex = 0;
        uint32_t location = 0;
        RenderFormat format = RenderFormat::UNKNOWN;
        uint32_t slotIndex = 0;
        uint32_t alignedByteOffset = 0;

        RenderInputElement() = default;

        RenderInputElement(const char *semanticName, uint32_t semanticIndex, uint32_t location, RenderFormat format, uint32_t slotIndex, uint32_t alignedByteOffset) {
            this->semanticName = semanticName;
            this->semanticIndex = semanticIndex;
            this->location = location;
            this->format = format;
            this->slotIndex = slotIndex;
            this->alignedByteOffset = alignedByteOffset;
        }
    };

    struct RenderBlendDesc {
        bool blendEnabled = false;
        RenderBlend srcBlend = RenderBlend::UNKNOWN;
        RenderBlend dstBlend = RenderBlend::UNKNOWN;
        RenderBlendOperation blendOp = RenderBlendOperation::UNKNOWN;
        RenderBlend srcBlendAlpha = RenderBlend::UNKNOWN;
        RenderBlend dstBlendAlpha = RenderBlend::UNKNOWN;
        RenderBlendOperation blendOpAlpha = RenderBlendOperation::UNKNOWN;
        uint8_t renderTargetWriteMask = uint8_t(RenderColorWriteEnable::ALL);

        static RenderBlendDesc Copy() {
            RenderBlendDesc desc;
            desc.srcBlend = RenderBlend::ONE;
            desc.dstBlend = RenderBlend::ZERO;
            desc.blendOp = RenderBlendOperation::ADD;
            desc.srcBlendAlpha = RenderBlend::ONE;
            desc.dstBlendAlpha = RenderBlend::ZERO;
            desc.blendOpAlpha = RenderBlendOperation::ADD;
            return desc;
        }

        static RenderBlendDesc AlphaBlend() {
            RenderBlendDesc desc;
            desc.blendEnabled = true;
            desc.srcBlend = RenderBlend::SRC_ALPHA;
            desc.dstBlend = RenderBlend::INV_SRC_ALPHA;
            desc.blendOp = RenderBlendOperation::ADD;
            desc.srcBlendAlpha = RenderBlend::SRC_ALPHA;
            desc.dstBlendAlpha = RenderBlend::INV_SRC_ALPHA;
            desc.blendOpAlpha = RenderBlendOperation::ADD;
            return desc;
        }
    };

    struct RenderSpecConstant {
        uint32_t index = 0;
        uint32_t value = 0;

        RenderSpecConstant() = default;

        RenderSpecConstant(uint32_t index, uint32_t value) {
            this->index = index;
            this->value = value;
        }
    };

    struct RenderComputePipelineDesc {
        const RenderPipelineLayout *pipelineLayout = nullptr;
        const RenderShader *computeShader = nullptr;
        const RenderSpecConstant *specConstants = nullptr;
        uint32_t specConstantsCount = 0;

        RenderComputePipelineDesc() = default;

        RenderComputePipelineDesc(const RenderPipelineLayout *pipelineLayout, const RenderShader *computeShader) {
            this->pipelineLayout = pipelineLayout;
            this->computeShader = computeShader;
        }
    };

    struct RenderGraphicsPipelineDesc {
        static const uint32_t MaxRenderTargets = 8;

        const RenderPipelineLayout *pipelineLayout = nullptr;
        const RenderShader *vertexShader = nullptr;
        const RenderShader *geometryShader = nullptr;
        const RenderShader *pixelShader = nullptr;
        RenderComparisonFunction depthFunction = RenderComparisonFunction::NEVER;
        bool depthClipEnabled = false;
        int32_t depthBias = 0;
        float slopeScaledDepthBias = 0.0f;
        bool dynamicDepthBiasEnabled = false;
        bool depthEnabled = false;
        bool depthWriteEnabled = false;
        RenderMultisampling multisampling;
        bool alphaToCoverageEnabled = false;
        RenderPrimitiveTopology primitiveTopology = RenderPrimitiveTopology::TRIANGLE_LIST;
        RenderCullMode cullMode = RenderCullMode::NONE;
        RenderFormat renderTargetFormat[MaxRenderTargets] = {};
        RenderBlendDesc renderTargetBlend[MaxRenderTargets] = {};
        uint32_t renderTargetCount = 0;
        bool logicOpEnabled = false;
        RenderLogicOperation logicOp = RenderLogicOperation::NOOP;
        RenderFormat depthTargetFormat = RenderFormat::UNKNOWN;
        const RenderInputSlot *inputSlots = nullptr;
        uint32_t inputSlotsCount = 0;
        const RenderInputElement *inputElements = nullptr;
        uint32_t inputElementsCount = 0;
        const RenderSpecConstant *specConstants = nullptr;
        uint32_t specConstantsCount = 0;
    };

    struct RenderRaytracingPipelineLibrarySymbol {
        const char *importName = nullptr;
        RenderRaytracingPipelineLibrarySymbolType type = RenderRaytracingPipelineLibrarySymbolType::UNKNOWN;
        const char *exportName = nullptr;
        const RenderSpecConstant *specConstants = nullptr;
        uint32_t specConstantsCount = 0;

        RenderRaytracingPipelineLibrarySymbol() = default;

        RenderRaytracingPipelineLibrarySymbol(const char *importName, RenderRaytracingPipelineLibrarySymbolType type, const char *exportName = nullptr, const RenderSpecConstant *specConstants = nullptr, uint32_t specConstantsCount = 0) {
            this->importName = importName;
            this->type = type;
            this->specConstants = specConstants;
            this->exportName = exportName;
            this->specConstantsCount = specConstantsCount;
        }
    };

    struct RenderRaytracingPipelineLibrary {
        const RenderShader *shader = nullptr;
        const RenderRaytracingPipelineLibrarySymbol *symbols = nullptr;
        uint32_t symbolsCount = 0;

        RenderRaytracingPipelineLibrary() = default;

        RenderRaytracingPipelineLibrary(const RenderShader *shader, const RenderRaytracingPipelineLibrarySymbol *symbols, uint32_t symbolsCount) {
            this->shader = shader;
            this->symbols = symbols;
            this->symbolsCount = symbolsCount;
        }
    };

    struct RenderRaytracingPipelineHitGroup {
        const char *hitGroupName = nullptr;
        const char *closestHitName = nullptr;
        const char *anyHitName = nullptr;
        const char *intersectionName = nullptr;

        RenderRaytracingPipelineHitGroup() = default;

        RenderRaytracingPipelineHitGroup(const char *hitGroupName, const char *closestHitName = nullptr, const char *anyHitName = nullptr, const char *intersectionName = nullptr) {
            this->hitGroupName = hitGroupName;
            this->closestHitName = closestHitName;
            this->anyHitName = anyHitName;
            this->intersectionName = intersectionName;
        }
    };

    struct RenderRaytracingPipelineDesc {
        const RenderRaytracingPipelineLibrary *libraries = nullptr;
        uint32_t librariesCount = 0;
        const RenderRaytracingPipelineHitGroup *hitGroups = nullptr;
        uint32_t hitGroupsCount = 0;
        const RenderPipelineLayout *pipelineLayout = nullptr;
        uint32_t maxPayloadSize = 0;
        uint32_t maxAttributeSize = 2 * sizeof(float);
        uint32_t maxRecursionDepth = 1;

        // IMPORTANT: State update support must be true for this option to work. The pipeline creation will not work if this option
        // is enabled and the device doesn't support it. This option is only supported by Raytracing Tier 1.1 devices.
        bool stateUpdateEnabled = false;
    };

    struct RenderPipelineProgram {
        uint32_t programIndex = 0;

        RenderPipelineProgram() = default;

        RenderPipelineProgram(uint32_t programIndex) {
            this->programIndex = programIndex;
        }
    };

    struct RenderSamplerDesc {
        RenderFilter minFilter = RenderFilter::LINEAR;
        RenderFilter magFilter = RenderFilter::LINEAR;
        RenderMipmapMode mipmapMode = RenderMipmapMode::LINEAR;
        RenderTextureAddressMode addressU = RenderTextureAddressMode::CLAMP;
        RenderTextureAddressMode addressV = RenderTextureAddressMode::CLAMP;
        RenderTextureAddressMode addressW = RenderTextureAddressMode::CLAMP;
        float mipLODBias = 0.0f;
        uint32_t maxAnisotropy = 16;
        bool anisotropyEnabled = false;
        RenderComparisonFunction comparisonFunc = RenderComparisonFunction::NEVER;
        bool comparisonEnabled = false;
        RenderBorderColor borderColor = RenderBorderColor::OPAQUE_BLACK;
        float minLOD = 0.0f;
        float maxLOD = FLT_MAX;
        RenderShaderVisibility shaderVisibility = RenderShaderVisibility::ALL;

        RenderSamplerDesc() = default;
    };

    struct RenderDescriptorRange {
        // The type of descriptor range. The descriptor can't change this during its lifetime.
        RenderDescriptorRangeType type = RenderDescriptorRangeType::UNKNOWN;

        // How many descriptors should be assigned and allocated for this range. When the range
        // is boundless (see RenderDescriptorSetDesc::lastRangeIsBoundless), this indicates the upper
        // bound of the variable sized array (TBD if this implies additional memory consumption).
        uint32_t count = 0;

        // The shader binding number the descriptor will correspond to.
        uint32_t binding = 0;

        // An optional immutable sampler to build in statically into the pipeline layout.
        const RenderSampler **immutableSampler = nullptr;

        RenderDescriptorRange() = default;

        RenderDescriptorRange(RenderDescriptorRangeType type, uint32_t binding, uint32_t count, const RenderSampler **immutableSampler = nullptr) {
            this->type = type;
            this->binding = binding;
            this->count = count;
            this->immutableSampler = immutableSampler;
        }
    };
    
    struct RenderDescriptorSetDesc {
        const RenderDescriptorRange *descriptorRanges = nullptr;
        uint32_t descriptorRangesCount = 0;
        bool lastRangeIsBoundless = false;
        uint32_t boundlessRangeSize = 0;

        RenderDescriptorSetDesc() = default;
        
        RenderDescriptorSetDesc(const RenderDescriptorRange *descriptorRanges, uint32_t descriptorRangesCount, bool lastRangeIsBoundless = false, uint32_t boundlessRangeSize = 0) {
            this->descriptorRanges = descriptorRanges;
            this->descriptorRangesCount = descriptorRangesCount;
            this->lastRangeIsBoundless = lastRangeIsBoundless;
            this->boundlessRangeSize = boundlessRangeSize;
        }
    };

    struct RenderPushConstantRange {
        uint32_t binding = 0;
        uint32_t set = 0;
        uint32_t offset = 0; // Must be aligned to 4-bytes for DX12.
        uint32_t size = 0;
        RenderShaderStageFlags stageFlags = RenderShaderStageFlag::NONE;

        RenderPushConstantRange() = default;

        RenderPushConstantRange(uint32_t binding, uint32_t set, uint32_t offset, uint32_t size, RenderShaderStageFlags stageFlags) {
            this->binding = binding;
            this->set = set;
            this->offset = offset;
            this->size = size;
            this->stageFlags = stageFlags;
        }
    };

    // D3D12 only.
    struct RenderRootDescriptorDesc {
        uint32_t shaderRegister = 0;
        uint32_t registerSpace = 0;
        RenderRootDescriptorType type = RenderRootDescriptorType::UNKNOWN;

        RenderRootDescriptorDesc() = default;

        RenderRootDescriptorDesc(uint32_t shaderRegister, uint32_t registerSpace, RenderRootDescriptorType type) {
            this->shaderRegister = shaderRegister;
            this->registerSpace = registerSpace;
            this->type = type;
        }
    };

    struct RenderPipelineLayoutDesc {
        const RenderPushConstantRange *pushConstantRanges = nullptr;
        uint32_t pushConstantRangesCount = 0;
        const RenderDescriptorSetDesc *descriptorSetDescs = nullptr;
        uint32_t descriptorSetDescsCount = 0;
        const RenderRootDescriptorDesc* rootDescriptorDescs = nullptr;
        uint32_t rootDescriptorDescsCount = 0;
        bool isLocal = false;
        bool allowInputLayout = false;

        RenderPipelineLayoutDesc() = default;

        RenderPipelineLayoutDesc(const RenderPushConstantRange *pushConstantRanges, uint32_t pushConstantRangesCount, const RenderDescriptorSetDesc *descriptorSetDescs, uint32_t descriptorSetDescsCount, bool isLocal = false, bool allowInputLayout = false) {
            this->pushConstantRanges = pushConstantRanges;
            this->pushConstantRangesCount = pushConstantRangesCount;
            this->descriptorSetDescs = descriptorSetDescs;
            this->descriptorSetDescsCount = descriptorSetDescsCount;
            this->isLocal = isLocal;
            this->allowInputLayout = allowInputLayout;
        }
    };

    struct RenderIndexBufferView {
        RenderBufferReference buffer;
        uint32_t size = 0;
        RenderFormat format = RenderFormat::UNKNOWN;

        RenderIndexBufferView() = default;

        RenderIndexBufferView(RenderBufferReference buffer, uint32_t size, RenderFormat format) {
            this->buffer = buffer;
            this->size = size;
            this->format = format;
        }
    };

    struct RenderVertexBufferView {
        RenderBufferReference buffer;
        uint32_t size = 0;

        RenderVertexBufferView() = default;

        RenderVertexBufferView(RenderBufferReference buffer, uint32_t size) {
            this->buffer = buffer;
            this->size = size;
        }
    };

    struct RenderSRV {
        RenderSRVType type = RenderSRVType::UNKNOWN;
        RenderFormat format = RenderFormat::UNKNOWN;

        union {
            struct {
                uint32_t firstElement;
                uint32_t structureByteStride;
                bool raw;
            } buffer;

            struct {
                uint32_t mipLevels;
            } texture;
        };

        RenderSRV() = default;

        RenderSRV(RenderSRVType type, RenderFormat format) {
            this->type = type;
            this->format = format;
        }

        static RenderSRV Buffer(RenderFormat format, uint32_t firstElement = 0, bool raw = false) {
            RenderSRV srv(RenderSRVType::BUFFER, format);
            srv.buffer.firstElement = firstElement;
            srv.buffer.structureByteStride = 0;
            srv.buffer.raw = raw;
            return srv;
        }

        static RenderSRV StructuredBuffer(uint32_t strideInBytes, uint32_t firstElement = 0, bool raw = false) {
            RenderSRV srv(RenderSRVType::BUFFER, RenderFormat::UNKNOWN);
            srv.buffer.firstElement = firstElement;
            srv.buffer.structureByteStride = strideInBytes;
            srv.buffer.raw = raw;
            return srv;
        }

        static RenderSRV Texture1D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipLevels = 1) {
            RenderSRV srv(RenderSRVType::TEXTURE_1D, format);
            srv.texture.mipLevels = mipLevels;
            return srv;
        }

        static RenderSRV Texture2D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipLevels = 1) {
            RenderSRV srv(RenderSRVType::TEXTURE_2D, format);
            srv.texture.mipLevels = mipLevels;
            return srv;
        }

        static RenderSRV Texture3D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipLevels = 1) {
            RenderSRV srv(RenderSRVType::TEXTURE_3D, format);
            srv.texture.mipLevels = mipLevels;
            return srv;
        }
    };

    struct RenderUAV {
        RenderUAVType type = RenderUAVType::UNKNOWN;
        RenderFormat format = RenderFormat::UNKNOWN;

        union {
            struct {
                uint32_t firstElement;
                uint32_t structureByteStride;
                bool raw;
            } buffer;

            struct {
                uint32_t mipSlice;
            } texture;
        };

        RenderUAV() = default;

        RenderUAV(RenderUAVType type, RenderFormat format) {
            this->type = type;
            this->format = format;
        }

        static RenderUAV Buffer(RenderFormat format, uint32_t firstElement = 0, bool raw = false) {
            RenderUAV uav(RenderUAVType::BUFFER, format);
            uav.buffer.firstElement = firstElement;
            uav.buffer.structureByteStride = 0;
            uav.buffer.raw = raw;
            return uav;
        }

        static RenderUAV StructuredBuffer(uint32_t strideInBytes, uint32_t firstElement = 0, bool raw = false) {
            RenderUAV uav(RenderUAVType::BUFFER, RenderFormat::UNKNOWN);
            uav.buffer.firstElement = firstElement;
            uav.buffer.structureByteStride = strideInBytes;
            uav.buffer.raw = raw;
            return uav;
        }

        static RenderUAV Texture1D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipSlice = 0) {
            RenderUAV uav(RenderUAVType::TEXTURE_1D, format);
            uav.texture.mipSlice = mipSlice;
            return uav;
        }

        static RenderUAV Texture2D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipSlice = 0) {
            RenderUAV uav(RenderUAVType::TEXTURE_2D, format);
            uav.texture.mipSlice = mipSlice;
            return uav;
        }

        static RenderUAV Texture3D(RenderFormat format = RenderFormat::UNKNOWN, uint32_t mipSlice = 0) {
            RenderUAV uav(RenderUAVType::TEXTURE_3D, format);
            uav.texture.mipSlice = mipSlice;
            return uav;
        }
    };

    struct RenderViewport {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        float minDepth = 0.0f;
        float maxDepth = 1.0f;

        RenderViewport() = default;

        RenderViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) {
            this->x = x;
            this->y = y;
            this->width = width;
            this->height = height;
            this->minDepth = minDepth;
            this->maxDepth = maxDepth;
        }

        bool operator==(const RenderViewport &v) const {
            return (x == v.x) && (y == v.y) && (width == v.width) && (height == v.height) && (minDepth == v.minDepth) && (maxDepth == v.maxDepth);
        }

        bool operator!=(const RenderViewport &v) const {
            return (x != v.x) || (y != v.y) || (width != v.width) || (height != v.height) || (minDepth != v.minDepth) || (maxDepth != v.maxDepth);
        }

        bool isEmpty() const {
            return (width <= 0.0f) || (height <= 0.0f);
        }
    };

    struct RenderRect {
        int32_t left = 0;
        int32_t top = 0;
        int32_t right = 0;
        int32_t bottom = 0;

        RenderRect() = default;

        RenderRect(int32_t left, int32_t top, int32_t right, int32_t bottom) {
            this->left = left;
            this->top = top;
            this->right = right;
            this->bottom = bottom;
        }

        bool operator==(const RenderRect &v) const {
            return (left == v.left) && (top == v.top) && (right == v.right) && (bottom == v.bottom);
        }

        bool operator!=(const RenderRect &v) const {
            return (left != v.left) || (top != v.top) || (right != v.right) || (bottom != v.bottom);
        }

        bool isEmpty() const {
            return (left >= right) || (top >= bottom);
        }
    };

    struct RenderBox {
        int32_t left = 0;
        int32_t top = 0;
        int32_t front = 0;
        int32_t right = 0;
        int32_t bottom = 0;
        int32_t back = 0;

        RenderBox() = default;

        RenderBox(int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t front = 0, int32_t back = 1) {
            this->left = left;
            this->top = top;
            this->front = front;
            this->right = right;
            this->bottom = bottom;
            this->back = back;
        }
    };

    struct RenderRange {
        uint64_t begin = 0;
        uint64_t end = 0;

        RenderRange() = default;

        RenderRange(uint64_t begin, uint64_t end) {
            this->begin = begin;
            this->end = end;
        }
    };

    struct RenderFramebufferDesc {
        const RenderTexture **colorAttachments = nullptr;
        uint32_t colorAttachmentsCount = 0;
        const RenderTexture *depthAttachment = nullptr;
        bool depthAttachmentReadOnly = false;

        RenderFramebufferDesc() = default;

        RenderFramebufferDesc(const RenderTexture **colorAttachments, uint32_t colorAttachmentsCount, const RenderTexture *depthAttachment = nullptr, bool depthAttachmentReadOnly = false) {
            this->colorAttachments = colorAttachments;
            this->colorAttachmentsCount = colorAttachmentsCount;
            this->depthAttachment = depthAttachment;
            this->depthAttachmentReadOnly = depthAttachmentReadOnly;
        }
    };

    struct RenderBottomLevelASMesh {
        RenderBufferReference indexBuffer;
        RenderBufferReference vertexBuffer;
        RenderFormat indexFormat = RenderFormat::UNKNOWN;
        RenderFormat vertexFormat = RenderFormat::UNKNOWN;
        uint32_t indexCount = 0;
        uint32_t vertexCount = 0;
        uint32_t vertexStride = 0;
        bool isOpaque = false;

        RenderBottomLevelASMesh() = default;

        RenderBottomLevelASMesh(RenderBufferReference indexBuffer, RenderBufferReference vertexBuffer, RenderFormat indexFormat, RenderFormat vertexFormat, uint32_t indexCount, uint32_t vertexCount, uint32_t vertexStride, bool isOpaque) {
            this->indexBuffer = indexBuffer;
            this->vertexBuffer = vertexBuffer;
            this->indexFormat = indexFormat;
            this->vertexFormat = vertexFormat;
            this->indexCount = indexCount;
            this->vertexCount = vertexCount;
            this->vertexStride = vertexStride;
            this->isOpaque = isOpaque;
        }
    };

    struct RenderBottomLevelASBuildInfo {
        uint32_t meshCount = 0;
        uint32_t primitiveCount = 0;
        bool preferFastBuild = false;
        bool preferFastTrace = false;
        uint64_t scratchSize = 0;
        uint64_t accelerationStructureSize = 0;

        // Private backend data. Can go unused.
        std::vector<uint8_t> buildData;
    };

    struct RenderTopLevelASInstance {
        RenderBufferReference bottomLevelAS;
        uint32_t instanceID = 0;
        uint32_t instanceMask = 0;
        uint32_t instanceContributionToHitGroupIndex = 0;
        bool cullDisable = false;
        RenderAffineTransform transform;

        RenderTopLevelASInstance() = default;

        RenderTopLevelASInstance(RenderBufferReference bottomLevelAS, uint32_t instanceID, uint32_t instanceMask, uint32_t instanceContributionToHitGroupIndex, bool cullDisable, RenderAffineTransform transform) {
            this->bottomLevelAS = bottomLevelAS;
            this->instanceID = instanceID;
            this->instanceMask = instanceMask;
            this->instanceContributionToHitGroupIndex = instanceContributionToHitGroupIndex;
            this->cullDisable = cullDisable;
            this->transform = transform;
        }
    };

    struct RenderTopLevelASBuildInfo {
        // The instances buffer data must be uploaded to the GPU by the API user.
        std::vector<uint8_t> instancesBufferData;
        uint32_t instanceCount = 0;
        bool preferFastBuild = false;
        bool preferFastTrace = false;
        uint64_t scratchSize = 0;
        uint64_t accelerationStructureSize = 0;

        // Private backend data. Can go unused.
        std::vector<uint8_t> buildData;
    };

    struct RenderShaderBindingGroup {
        const RenderPipelineProgram *pipelinePrograms = nullptr;
        uint32_t pipelineProgramsCount = 0;

        RenderShaderBindingGroup() = default;

        RenderShaderBindingGroup(const RenderPipelineProgram *pipelinePrograms, uint32_t pipelineProgramsCount) {
            this->pipelinePrograms = pipelinePrograms;
            this->pipelineProgramsCount = pipelineProgramsCount;
        }
    };

    struct RenderShaderBindingGroups {
        RenderShaderBindingGroup rayGen;
        RenderShaderBindingGroup miss;
        RenderShaderBindingGroup hitGroup;
        RenderShaderBindingGroup callable;

        RenderShaderBindingGroups() = default;

        RenderShaderBindingGroups(RenderShaderBindingGroup rayGen, RenderShaderBindingGroup miss, RenderShaderBindingGroup hitGroup, RenderShaderBindingGroup callable = RenderShaderBindingGroup()) {
            this->rayGen = rayGen;
            this->miss = miss;
            this->hitGroup = hitGroup;
            this->callable = callable;
        }
    };

    struct RenderShaderBindingGroupInfo {
        uint64_t offset = 0;
        uint64_t size = 0;
        uint32_t stride = 0;

        // Convenience index for selecting a different binding in the table. offset must add startIndex * stride.
        uint32_t startIndex = 0;
    };

    struct RenderShaderBindingGroupsInfo {
        RenderShaderBindingGroupInfo rayGen;
        RenderShaderBindingGroupInfo miss;
        RenderShaderBindingGroupInfo hitGroup;
        RenderShaderBindingGroupInfo callable;
    };

    struct RenderShaderBindingTableInfo {
        // The table buffer data must be uploaded to the GPU by the API user and submitted to dispatchRays().
        std::vector<uint8_t> tableBufferData;

        // This info will be requested by dispatchRays().
        RenderShaderBindingGroupsInfo groups;
    };

    struct RenderDeviceDescription {
        std::string name = "Unknown";
        RenderDeviceType type = RenderDeviceType::UNKNOWN;
        uint32_t driverVersion = 0;
        uint64_t dedicatedVideoMemory = 0;
    };

    struct RenderDeviceCapabilities {
        // Raytracing.
        bool raytracing = false;
        bool raytracingStateUpdate = false;

        // MSAA.
        bool sampleLocations = false;

        // Bindless resources.
        bool descriptorIndexing = false;
        bool scalarBlockLayout = false;

        // Present.
        bool presentWait = false;
        bool displayTiming = false;

        // HDR.
        bool preferHDR = false;

        // Draw.
        bool triangleFan = false;
        bool dynamicDepthBias = false;

        // UMA.
        bool uma = false;
    };

    struct RenderInterfaceCapabilities {
        RenderShaderFormat shaderFormat = RenderShaderFormat::UNKNOWN;
    };
};
