#include <stdafx.h>

#include <kernel/function.h>
#include <kernel/heap.h>
#include <cpu/code_cache.h>
#include <cpu/guest_code.h>
#include <cpu/guest_thread.h>
#include <kernel/memory.h>
#include <kernel/xdbf.h>
#include <xxHashMap.h>
#include <shader/shader_cache.h>
#include <ui/achievement_menu.h>
#include <ui/achievement_overlay.h>
#include <ui/button_guide.h>
#include <ui/fader.h>
#include <ui/message_window.h>
#include <ui/options_menu.h>
#include <ui/installer_wizard.h>

#include "imgui_snapshot.h"
#include "imgui_common.h"
#include "video.h"
#include <ui/sdl_listener.h>
#include <ui/window.h>
#include <user/config.h>

#include <res/font/im_font_atlas.dds.h>
#include <res/bc_diff/button_bc_diff.bin.h>
#include <decompressor.h>
#include <bc_diff.h>

#include <SWA.h>

#include "../../thirdparty/ShaderRecomp/ShaderRecomp/shader_common.h"
#include "shader/copy_vs.hlsl.dxil.h"
#include "shader/copy_vs.hlsl.spirv.h"
#include "shader/gaussian_blur_3x3.hlsl.dxil.h"
#include "shader/gaussian_blur_3x3.hlsl.spirv.h"
#include "shader/gaussian_blur_5x5.hlsl.dxil.h"
#include "shader/gaussian_blur_5x5.hlsl.spirv.h"
#include "shader/gaussian_blur_7x7.hlsl.dxil.h"
#include "shader/gaussian_blur_7x7.hlsl.spirv.h"
#include "shader/gaussian_blur_9x9.hlsl.dxil.h"
#include "shader/gaussian_blur_9x9.hlsl.spirv.h"
#include "shader/gamma_correction_ps.hlsl.dxil.h"
#include "shader/gamma_correction_ps.hlsl.spirv.h"
#include "shader/imgui_ps.hlsl.dxil.h"
#include "shader/imgui_ps.hlsl.spirv.h"
#include "shader/imgui_vs.hlsl.dxil.h"
#include "shader/imgui_vs.hlsl.spirv.h"
#include "shader/movie_vs.hlsl.dxil.h"
#include "shader/movie_vs.hlsl.spirv.h"
#include "shader/movie_ps.hlsl.dxil.h"
#include "shader/movie_ps.hlsl.spirv.h"
#include "shader/resolve_msaa_depth_2x.hlsl.dxil.h"
#include "shader/resolve_msaa_depth_2x.hlsl.spirv.h"
#include "shader/resolve_msaa_depth_4x.hlsl.dxil.h"
#include "shader/resolve_msaa_depth_4x.hlsl.spirv.h"
#include "shader/resolve_msaa_depth_8x.hlsl.dxil.h"
#include "shader/resolve_msaa_depth_8x.hlsl.spirv.h"

#if defined(ASYNC_PSO_DEBUG) || defined(PSO_CACHING)
#include <magic_enum.hpp>
#endif

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

namespace plume
{
    extern std::unique_ptr<RenderInterface> CreateD3D12Interface();
    extern std::unique_ptr<RenderInterface> CreateVulkanInterface();
}

#pragma pack(push, 1)
struct PipelineState
{
    GuestShader* vertexShader = nullptr;
    GuestShader* pixelShader = nullptr;
    GuestVertexDeclaration* vertexDeclaration = nullptr;
    bool instancing = false;
    bool zEnable = true;
    bool zWriteEnable = true;
    RenderBlend srcBlend = RenderBlend::ONE;
    RenderBlend destBlend = RenderBlend::ZERO;
    RenderCullMode cullMode = RenderCullMode::NONE;
    RenderComparisonFunction zFunc = RenderComparisonFunction::LESS;
    bool alphaBlendEnable = false;
    RenderBlendOperation blendOp = RenderBlendOperation::ADD;
    float slopeScaledDepthBias = 0.0f;
    int32_t depthBias = 0;
    RenderBlend srcBlendAlpha = RenderBlend::ONE;
    RenderBlend destBlendAlpha = RenderBlend::ZERO;
    RenderBlendOperation blendOpAlpha = RenderBlendOperation::ADD;
    uint32_t colorWriteEnable = uint32_t(RenderColorWriteEnable::ALL);
    RenderPrimitiveTopology primitiveTopology = RenderPrimitiveTopology::TRIANGLE_LIST;
    uint8_t vertexStrides[16]{};
    RenderFormat renderTargetFormat{};
    RenderFormat depthStencilFormat{};
    RenderSampleCounts sampleCount = RenderSampleCount::COUNT_1;
    bool enableAlphaToCoverage = false;
    uint32_t specConstants = 0;
};
#pragma pack(pop)

struct SharedConstants
{
    uint32_t texture2DIndices[16]{};
    uint32_t texture3DIndices[16]{};
    uint32_t textureCubeIndices[16]{};
    uint32_t samplerIndices[16]{};
    uint32_t booleans{};
    uint32_t swappedTexcoords{};
    float alphaThreshold{};
};

static GuestSurface* g_renderTarget;
static GuestSurface* g_depthStencil;
static RenderFramebuffer* g_framebuffer;
static RenderViewport g_viewport(0.0f, 0.0f, 1280.0f, 720.0f);
static bool g_halfPixel = true;
static PipelineState g_pipelineState;
static SharedConstants g_sharedConstants;
static RenderSamplerDesc g_samplerDescs[16];
static bool g_scissorTestEnable = false;
static RenderRect g_scissorRect;
static RenderVertexBufferView g_vertexBufferViews[16];
static RenderInputSlot g_inputSlots[16];
static RenderIndexBufferView g_indexBufferView({}, 0, RenderFormat::R16_UINT);

struct DirtyStates
{
    bool renderTargetAndDepthStencil;
    bool viewport;
    bool pipelineState;
    bool sharedConstants;
    bool scissorRect;
    bool vertexShaderConstants;
    uint8_t vertexStreamFirst;
    uint8_t vertexStreamLast;
    bool indices;
    bool pixelShaderConstants;

    DirtyStates(bool value)
        : renderTargetAndDepthStencil(value)
        , viewport(value)
        , pipelineState(value)
        , sharedConstants(value)
        , scissorRect(value)
        , vertexShaderConstants(value)
        , vertexStreamFirst(value ? 0 : 255)
        , vertexStreamLast(value ? 15 : 0)
        , indices(value)
        , pixelShaderConstants(value)
    {
    }
};

static DirtyStates g_dirtyStates(true);

template<typename T>
static FORCEINLINE void SetDirtyValue(bool& dirtyState, T& dest, const T& src)
{
    if (dest != src)
    {
        dest = src;
        dirtyState = true;
    }
}

static bool g_vulkan;
static std::unique_ptr<RenderInterface> g_interface;
static std::unique_ptr<RenderDevice> g_device;

static bool g_triangleFanSupported;

static constexpr size_t NUM_FRAMES = 2;

static uint32_t g_frame = 0;
static uint32_t g_nextFrame = 1;

static std::unique_ptr<RenderCommandQueue> g_queue;
static std::unique_ptr<RenderCommandList> g_commandLists[NUM_FRAMES];
static std::unique_ptr<RenderCommandFence> g_commandFences[NUM_FRAMES];
static bool g_commandListStates[NUM_FRAMES];

static Mutex g_copyMutex;
static std::unique_ptr<RenderCommandQueue> g_copyQueue;
static std::unique_ptr<RenderCommandList> g_copyCommandList;
static std::unique_ptr<RenderCommandFence> g_copyCommandFence;

static std::unique_ptr<RenderSwapChain> g_swapChain;
static bool g_swapChainValid;
static bool g_needsResize;

static constexpr RenderFormat BACKBUFFER_FORMAT = RenderFormat::B8G8R8A8_UNORM;

static std::unique_ptr<RenderCommandSemaphore> g_acquireSemaphores[NUM_FRAMES];
static std::unique_ptr<RenderCommandSemaphore> g_renderSemaphores[NUM_FRAMES];
static uint32_t g_backBufferIndex;
static GuestSurface* g_backBuffer;

static std::unique_ptr<RenderTexture> g_intermediaryBackBufferTexture;
static uint32_t g_intermediaryBackBufferTextureWidth;
static uint32_t g_intermediaryBackBufferTextureHeight;
static uint32_t g_intermediaryBackBufferTextureDescriptorIndex;

static std::unique_ptr<RenderPipeline> g_gammaCorrectionPipeline;

struct std::unique_ptr<RenderDescriptorSet> g_textureDescriptorSet;
struct std::unique_ptr<RenderDescriptorSet> g_samplerDescriptorSet;

enum
{
    TEXTURE_DESCRIPTOR_NULL_TEXTURE_2D,
    TEXTURE_DESCRIPTOR_NULL_TEXTURE_3D,
    TEXTURE_DESCRIPTOR_NULL_TEXTURE_CUBE,
    TEXTURE_DESCRIPTOR_NULL_COUNT
};

struct TextureDescriptorAllocator
{
    Mutex mutex;
    uint32_t capacity = TEXTURE_DESCRIPTOR_NULL_COUNT;
    std::vector<uint32_t> freed;

    uint32_t allocate()
    {
        std::lock_guard lock(mutex);

        uint32_t value;
        if (!freed.empty())
        {
            value = freed.back();
            freed.pop_back();
        }
        else
        {
            value = capacity;
            ++capacity;
        }

        return value;
    }

    void free(uint32_t value)
    {
        assert(value != NULL);
        std::lock_guard lock(mutex);
        freed.push_back(value);
    }
};

static std::unique_ptr<RenderTexture> g_blankTextures[TEXTURE_DESCRIPTOR_NULL_COUNT];
static std::unique_ptr<RenderTextureView> g_blankTextureViews[TEXTURE_DESCRIPTOR_NULL_COUNT];

static TextureDescriptorAllocator g_textureDescriptorAllocator;

static std::unique_ptr<RenderPipelineLayout> g_pipelineLayout;
static xxHashMap<std::unique_ptr<RenderPipeline>> g_pipelines;

#ifdef ASYNC_PSO_DEBUG
static std::atomic<uint32_t> g_pipelinesCreatedInRenderThread;
static std::atomic<uint32_t> g_pipelinesCreatedAsynchronously;
static std::atomic<uint32_t> g_pipelinesDropped;
static std::atomic<uint32_t> g_pipelinesCurrentlyCompiling;
static std::string g_pipelineDebugText;
static Mutex g_debugMutex;
#endif

#ifdef PSO_CACHING
static std::vector<PipelineState> g_pipelineStatesToCache;
static Mutex g_pipelineCacheMutex;
#endif

static std::atomic<uint32_t> g_compilingDataCount;
static std::atomic<uint32_t> g_pendingDataCount;

static const PipelineState g_pipelineStateCache[] =
{
#include "cache/pipeline_state_cache.h"
};

static bool g_pendingPipelineStateCache = true;

#include "cache/vertex_element_cache.h"

static uint8_t* const g_vertexDeclarationCache[] =
{
#include "cache/vertex_declaration_cache.h"
};

static xxHashMap<std::pair<uint32_t, std::unique_ptr<RenderSampler>>> g_samplerStates;

static Mutex g_vertexDeclarationMutex;
static xxHashMap<GuestVertexDeclaration*> g_vertexDeclarations;

struct UploadBuffer
{
    static constexpr size_t SIZE = 16 * 1024 * 1024;

    std::unique_ptr<RenderBuffer> buffer;
    uint8_t* memory = nullptr;
    uint64_t deviceAddress = 0;
};

struct UploadAllocation
{
    const RenderBuffer* buffer;
    uint64_t offset;
    uint8_t* memory;
    uint64_t deviceAddress;
};

struct UploadAllocator
{
    std::vector<UploadBuffer> buffers;
    uint32_t index = 0;
    uint32_t offset = 0;
    Mutex mutex;

    UploadAllocation allocate(uint32_t size, uint32_t alignment)
    {
        std::lock_guard lock(mutex);

        assert(size <= UploadBuffer::SIZE);

        offset = (offset + alignment - 1) & ~(alignment - 1);

        if (offset + size > UploadBuffer::SIZE)
        {
            ++index;
            offset = 0;
        }

        if (buffers.size() <= index)
            buffers.resize(index + 1);

        auto& buffer = buffers[index];
        if (buffer.buffer == nullptr)
        {
            buffer.buffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(UploadBuffer::SIZE, RenderBufferFlag::CONSTANT | RenderBufferFlag::VERTEX | RenderBufferFlag::INDEX));
            buffer.memory = reinterpret_cast<uint8_t*>(buffer.buffer->map());
            buffer.deviceAddress = buffer.buffer->getDeviceAddress();
        }
        
        auto ref = buffer.buffer->at(offset);
        offset += size;

        return { ref.ref, ref.offset, buffer.memory + ref.offset, buffer.deviceAddress + ref.offset };
    }

    template<bool TByteSwap, typename T>
    UploadAllocation allocate(const T* memory, uint32_t size, uint32_t alignment)
    {
        auto result = allocate(size, alignment);

        if constexpr (TByteSwap)
        {
            auto destination = reinterpret_cast<T*>(result.memory);

            for (size_t i = 0; i < size; i += sizeof(T))
            {
                *destination = std::byteswap(*memory);
                ++destination;
                ++memory;
            }
        }
        else
        {
            memcpy(result.memory, memory, size);
        }

        return result;
    }

    void reset()
    {
        index = 0;
        offset = 0;
    }
};

static UploadAllocator g_uploadAllocators[NUM_FRAMES];

static std::vector<GuestResource*> g_tempResources[NUM_FRAMES];
static std::vector<std::unique_ptr<RenderBuffer>> g_tempBuffers[NUM_FRAMES];

template<GuestPrimitiveType PrimitiveType>
struct PrimitiveIndexData
{
    std::vector<uint16_t> indexData;
    RenderBufferReference indexBuffer;
    uint32_t currentIndexCount = 0;

    uint32_t prepare(uint32_t guestPrimCount)
    {
        uint32_t primCount;
        uint32_t indexCountPerPrimitive;

        switch (PrimitiveType)
        {
        case D3DPT_TRIANGLEFAN:
            primCount = guestPrimCount - 2;
            indexCountPerPrimitive = 3; 
            break;
        case D3DPT_QUADLIST:
            primCount = guestPrimCount / 4;
            indexCountPerPrimitive = 6;
            break;
        default:
            assert(false && "Unknown primitive type.");
            break;
        }

        uint32_t indexCount = primCount * indexCountPerPrimitive;

        if (indexData.size() < indexCount)
        {
            const size_t oldPrimCount = indexData.size() / indexCountPerPrimitive;
            indexData.resize(indexCount);

            for (size_t i = oldPrimCount; i < primCount; i++)
            {
                switch (PrimitiveType)
                {
                case D3DPT_TRIANGLEFAN:
                {
                    indexData[i * 3 + 0] = 0;
                    indexData[i * 3 + 1] = static_cast<uint16_t>(i + 1);
                    indexData[i * 3 + 2] = static_cast<uint16_t>(i + 2);
                    break;
                }
                case D3DPT_QUADLIST:
                {
                    indexData[i * 6 + 0] = static_cast<uint16_t>(i * 4 + 0);
                    indexData[i * 6 + 1] = static_cast<uint16_t>(i * 4 + 1);
                    indexData[i * 6 + 2] = static_cast<uint16_t>(i * 4 + 2);

                    indexData[i * 6 + 3] = static_cast<uint16_t>(i * 4 + 0);
                    indexData[i * 6 + 4] = static_cast<uint16_t>(i * 4 + 2);
                    indexData[i * 6 + 5] = static_cast<uint16_t>(i * 4 + 3);
                    break;
                }
                default:
                    assert(false && "Unknown primitive type.");
                    break;
                }
            }
        }

        if (indexBuffer == NULL || currentIndexCount < indexCount)
        {
            auto allocation = g_uploadAllocators[g_frame].allocate<false>(indexData.data(), indexCount * 2, 2);
            indexBuffer = allocation.buffer->at(allocation.offset);
            currentIndexCount = indexCount;
        }

        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.buffer, indexBuffer);
        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.size, indexCount * 2);
        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.format, RenderFormat::R16_UINT);

        return indexCount;
    }

    void reset()
    {
        indexBuffer = {};
        currentIndexCount = 0;
    }
};

static PrimitiveIndexData<D3DPT_TRIANGLEFAN> g_triangleFanIndexData;
static PrimitiveIndexData<D3DPT_QUADLIST> g_quadIndexData;

static void DestructTempResources()
{
    for (auto resource : g_tempResources[g_frame])
    {
        switch (resource->type)
        {
        case ResourceType::Texture:
        case ResourceType::VolumeTexture:
        {
            const auto texture = reinterpret_cast<GuestTexture*>(resource);

            if (texture->mappedMemory != nullptr)
                g_userHeap.Free(texture->mappedMemory);

            g_textureDescriptorAllocator.free(texture->descriptorIndex);

            if (texture->patchedTexture != nullptr)
                g_textureDescriptorAllocator.free(texture->patchedTexture->descriptorIndex);

            texture->~GuestTexture();
            break;
        }

        case ResourceType::VertexBuffer:
        case ResourceType::IndexBuffer:
        {
            const auto buffer = reinterpret_cast<GuestBuffer*>(resource);

            if (buffer->mappedMemory != nullptr)
                g_userHeap.Free(buffer->mappedMemory);

            buffer->~GuestBuffer();
            break;
        }

        case ResourceType::RenderTarget:
        case ResourceType::DepthStencil:
        {
            const auto surface = reinterpret_cast<GuestSurface*>(resource);

            if (surface->descriptorIndex != NULL)
                g_textureDescriptorAllocator.free(surface->descriptorIndex);

            surface->~GuestSurface();
            break;
        }

        case ResourceType::VertexDeclaration:
            reinterpret_cast<GuestVertexDeclaration*>(resource)->~GuestVertexDeclaration();
            break;

        case ResourceType::VertexShader:
        case ResourceType::PixelShader:
        {
            reinterpret_cast<GuestShader*>(resource)->~GuestShader();
            break;
        }
        }

        g_userHeap.Free(resource);
    }

    g_tempResources[g_frame].clear();
    g_tempBuffers[g_frame].clear();
}

static uint32_t g_mainThreadId;

static ankerl::unordered_dense::map<RenderTexture*, RenderTextureLayout> g_barrierMap;

static void AddBarrier(GuestBaseTexture* texture, RenderTextureLayout layout)
{
    if (texture != nullptr && texture->layout != layout)
    {
        g_barrierMap[texture->texture] = layout;
        texture->layout = layout;
    }
}

static std::vector<RenderTextureBarrier> g_barriers;

static void FlushBarriers()
{
    if (!g_barrierMap.empty())
    {
        for (auto& [texture, layout] : g_barrierMap)
            g_barriers.emplace_back(texture, layout);

        g_commandLists[g_frame]->barriers(RenderBarrierStage::GRAPHICS | RenderBarrierStage::COPY, g_barriers);

        g_barrierMap.clear();
        g_barriers.clear();
    }
}

static std::unique_ptr<uint8_t[]> g_shaderCache;
static std::unique_ptr<uint8_t[]> g_buttonBcDiff;

static void LoadEmbeddedResources()
{
    const size_t decompressedSize = g_vulkan ? g_spirvCacheDecompressedSize : g_dxilCacheDecompressedSize;
    g_shaderCache = std::make_unique<uint8_t[]>(decompressedSize);

    ZSTD_decompress(g_shaderCache.get(), 
        decompressedSize, 
        g_vulkan ? g_compressedSpirvCache : g_compressedDxilCache, 
        g_vulkan ? g_spirvCacheCompressedSize : g_dxilCacheCompressedSize);

    g_buttonBcDiff = decompressZstd(g_button_bc_diff, g_button_bc_diff_uncompressed_size);
}

enum class RenderCommandType
{
    SetRenderState,
    DestructResource,
    UnlockTextureRect,
    UnlockBuffer16,
    UnlockBuffer32,
    DrawImGui,
    Present,
    StretchRect,
    SetRenderTarget,
    SetDepthStencilSurface,
    Clear,
    SetViewport,
    SetTexture,
    SetScissorRect,
    SetSamplerState,
    SetBooleans,
    SetVertexShaderConstants,
    SetPixelShaderConstants,
    AddPipeline,
    DrawPrimitive,
    DrawIndexedPrimitive,
    DrawPrimitiveUP,
    SetVertexDeclaration,
    SetVertexShader,
    SetStreamSource,
    SetIndices,
    SetPixelShader
};

struct RenderCommand
{
    RenderCommandType type;
    union
    {
        struct
        {
            GuestRenderState type;
            uint32_t value;
        } setRenderState;

        struct 
        {
            GuestResource* resource;
        } destructResource;

        struct
        {
            GuestTexture* texture;
        } unlockTextureRect;

        struct
        {
            GuestBuffer* buffer;
        } unlockBuffer;

        struct 
        {
            GuestDevice* device;
            uint32_t flags;
            GuestTexture* texture;
        } stretchRect;

        struct 
        {
            GuestSurface* renderTarget;
        } setRenderTarget;

        struct 
        {
            GuestSurface* depthStencil;
        } setDepthStencilSurface;

        struct 
        {
            uint32_t flags;
            float color[4];
            float z;
        } clear;

        struct 
        {
            float x;
            float y;
            float width;
            float height;
            float minDepth;
            float maxDepth;
        } setViewport;

        struct 
        {
            uint32_t index;
            GuestTexture* texture;
        } setTexture;

        struct 
        {
            int32_t left;
            int32_t top;
            int32_t right;
            int32_t bottom;
        } setScissorRect;

        struct
        {
            uint32_t index;
            uint32_t data0;
            uint32_t data3;
            uint32_t data5;
        } setSamplerState;

        struct
        {
            uint32_t booleans;
        } setBooleans;

        struct
        {
            UploadAllocation allocation;
        } setVertexShaderConstants;  
        
        struct
        {
            UploadAllocation allocation;
        } setPixelShaderConstants;

        struct
        {
            XXH64_hash_t hash;
            RenderPipeline* pipeline;
        } addPipeline;

        struct 
        {
            uint32_t primitiveType; 
            uint32_t startVertex; 
            uint32_t primitiveCount;
        } drawPrimitive;

        struct 
        {
            uint32_t primitiveType;
            int32_t baseVertexIndex; 
            uint32_t startIndex;
            uint32_t primCount;
        } drawIndexedPrimitive;

        struct 
        {
            uint32_t primitiveType;
            uint32_t primitiveCount; 
            UploadAllocation vertexStreamZeroData; 
            uint32_t vertexStreamZeroStride;
        } drawPrimitiveUP;

        struct 
        {
            GuestVertexDeclaration* vertexDeclaration;
        } setVertexDeclaration;

        struct 
        {
            GuestShader* shader;
        } setVertexShader;

        struct 
        {
            uint32_t index;
            GuestBuffer* buffer;
            uint32_t offset;
            uint32_t stride;
        } setStreamSource;

        struct 
        {
            GuestBuffer* buffer;
        } setIndices;

        struct 
        {
            GuestShader* shader;
        } setPixelShader;
    };
};

static moodycamel::BlockingConcurrentQueue<RenderCommand> g_renderQueue;

template<GuestRenderState TType>
static void SetRenderState(GuestDevice* device, uint32_t value)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetRenderState;
    cmd.setRenderState.type = TType;
    cmd.setRenderState.value = value;
    g_renderQueue.enqueue(cmd);
}

static void SetRenderStateUnimplemented(GuestDevice* device, uint32_t value)
{
}

static void SetAlphaTestMode(bool enable)
{
    uint32_t specConstants = 0;
    bool enableAlphaToCoverage = false;

    if (enable)
    {
        enableAlphaToCoverage = Config::TransparencyAntiAliasing && g_renderTarget != nullptr && g_renderTarget->sampleCount != RenderSampleCount::COUNT_1;

        if (enableAlphaToCoverage)
            specConstants = SPEC_CONSTANT_ALPHA_TO_COVERAGE;
        else
            specConstants = SPEC_CONSTANT_ALPHA_TEST;
    }

    specConstants |= (g_pipelineState.specConstants & ~(SPEC_CONSTANT_ALPHA_TEST | SPEC_CONSTANT_ALPHA_TO_COVERAGE));

    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.enableAlphaToCoverage, enableAlphaToCoverage);
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.specConstants, specConstants);
}

static RenderBlend ConvertBlendMode(uint32_t blendMode)
{
    switch (blendMode)
    {
    case D3DBLEND_ZERO:
        return RenderBlend::ZERO;
    case D3DBLEND_ONE:
        return RenderBlend::ONE;
    case D3DBLEND_SRCCOLOR:
        return RenderBlend::SRC_COLOR;
    case D3DBLEND_INVSRCCOLOR:
        return RenderBlend::INV_SRC_COLOR;
    case D3DBLEND_SRCALPHA:
        return RenderBlend::SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA:
        return RenderBlend::INV_SRC_ALPHA;
    case D3DBLEND_DESTCOLOR:
        return RenderBlend::DEST_COLOR;
    case D3DBLEND_INVDESTCOLOR:
        return RenderBlend::INV_DEST_COLOR;
    case D3DBLEND_DESTALPHA:
        return RenderBlend::DEST_ALPHA;
    case D3DBLEND_INVDESTALPHA:
        return RenderBlend::INV_DEST_ALPHA;
    default:
        assert(false && "Invalid blend mode");
        return RenderBlend::ZERO;
    }
}

static RenderBlendOperation ConvertBlendOp(uint32_t blendOp)
{
    switch (blendOp)
    {
    case D3DBLENDOP_ADD:
        return RenderBlendOperation::ADD;
    case D3DBLENDOP_SUBTRACT:
        return RenderBlendOperation::SUBTRACT;
    case D3DBLENDOP_REVSUBTRACT:
        return RenderBlendOperation::REV_SUBTRACT;
    case D3DBLENDOP_MIN:
        return RenderBlendOperation::MIN;
    case D3DBLENDOP_MAX:
        return RenderBlendOperation::MAX;
    default:
        assert(false && "Unknown blend operation");
        return RenderBlendOperation::ADD;
    }
}

static void ProcSetRenderState(const RenderCommand& cmd)
{
    uint32_t value = cmd.setRenderState.value;

    switch (cmd.setRenderState.type)
    {
    case D3DRS_ZENABLE:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.zEnable, value != 0);
        g_dirtyStates.renderTargetAndDepthStencil |= g_dirtyStates.pipelineState;
        break;
    }
    case D3DRS_ZWRITEENABLE:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.zWriteEnable, value != 0);
        break;
    }
    case D3DRS_ALPHATESTENABLE:
    {
        SetAlphaTestMode(value != 0);
        break;
    }
    case D3DRS_SRCBLEND:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.srcBlend, ConvertBlendMode(value));
        break;
    }
    case D3DRS_DESTBLEND:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.destBlend, ConvertBlendMode(value));
        break;
    }
    case D3DRS_CULLMODE:
    {
        RenderCullMode cullMode;

        switch (value) {
        case D3DCULL_NONE:
        case D3DCULL_NONE_2:
            cullMode = RenderCullMode::NONE;
            break;
        case D3DCULL_CW:
            cullMode = RenderCullMode::FRONT;
            break;
        case D3DCULL_CCW:
            cullMode = RenderCullMode::BACK;
            break;
        default:
            assert(false && "Invalid cull mode");
            cullMode = RenderCullMode::NONE;
            break;
        }

        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.cullMode, cullMode);
        break;
    }
    case D3DRS_ZFUNC:
    {
        RenderComparisonFunction comparisonFunc;

        switch (value)
        {
        case D3DCMP_NEVER:
            comparisonFunc = RenderComparisonFunction::NEVER;
            break;
        case D3DCMP_LESS:
            comparisonFunc = RenderComparisonFunction::LESS;
            break;
        case D3DCMP_EQUAL:
            comparisonFunc = RenderComparisonFunction::EQUAL;
            break;
        case D3DCMP_LESSEQUAL:
            comparisonFunc = RenderComparisonFunction::LESS_EQUAL;
            break;
        case D3DCMP_GREATER:
            comparisonFunc = RenderComparisonFunction::GREATER;
            break;
        case D3DCMP_NOTEQUAL:
            comparisonFunc = RenderComparisonFunction::NOT_EQUAL;
            break;
        case D3DCMP_GREATEREQUAL:
            comparisonFunc = RenderComparisonFunction::GREATER_EQUAL;
            break;
        case D3DCMP_ALWAYS:
            comparisonFunc = RenderComparisonFunction::ALWAYS;
            break;
        default:
            assert(false && "Unknown comparison function");
            comparisonFunc = RenderComparisonFunction::NEVER;
            break;
        }

        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.zFunc, comparisonFunc);
        break;
    }
    case D3DRS_ALPHAREF:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_sharedConstants.alphaThreshold, float(value) / 256.0f);
        break;
    }
    case D3DRS_ALPHABLENDENABLE:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.alphaBlendEnable, value != 0);
        break;
    }
    case D3DRS_BLENDOP:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.blendOp, ConvertBlendOp(value));
        break;
    }
    case D3DRS_SCISSORTESTENABLE:
    {
        SetDirtyValue(g_dirtyStates.scissorRect, g_scissorTestEnable, value != 0);
        break;
    }
    case D3DRS_SLOPESCALEDEPTHBIAS:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.slopeScaledDepthBias, *reinterpret_cast<float*>(&value));
        break;
    }
    case D3DRS_DEPTHBIAS:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.depthBias, int32_t(*reinterpret_cast<float*>(&value) * (1 << 24)));
        break;
    }
    case D3DRS_SRCBLENDALPHA:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.srcBlendAlpha, ConvertBlendMode(value));
        break;
    }
    case D3DRS_DESTBLENDALPHA:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.destBlendAlpha, ConvertBlendMode(value));
        break;
    }
    case D3DRS_BLENDOPALPHA:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.blendOpAlpha, ConvertBlendOp(value));
        break;
    }
    case D3DRS_COLORWRITEENABLE:
    {
        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.colorWriteEnable, value);
        g_dirtyStates.renderTargetAndDepthStencil |= g_dirtyStates.pipelineState;
        break;
    }
    }
}

static const std::pair<GuestRenderState, void*> g_setRenderStateFunctions[] =
{
    { D3DRS_ZENABLE, HostToGuestFunction<SetRenderState<D3DRS_ZENABLE>> },
    { D3DRS_ZWRITEENABLE, HostToGuestFunction<SetRenderState<D3DRS_ZWRITEENABLE>> },
    { D3DRS_ALPHATESTENABLE, HostToGuestFunction<SetRenderState<D3DRS_ALPHATESTENABLE>> },
    { D3DRS_SRCBLEND, HostToGuestFunction<SetRenderState<D3DRS_SRCBLEND>> },
    { D3DRS_DESTBLEND, HostToGuestFunction<SetRenderState<D3DRS_DESTBLEND>> },
    { D3DRS_CULLMODE, HostToGuestFunction<SetRenderState<D3DRS_CULLMODE>> },
    { D3DRS_ZFUNC, HostToGuestFunction<SetRenderState<D3DRS_ZFUNC>> },
    { D3DRS_ALPHAREF, HostToGuestFunction<SetRenderState<D3DRS_ALPHAREF>> },
    { D3DRS_ALPHABLENDENABLE, HostToGuestFunction<SetRenderState<D3DRS_ALPHABLENDENABLE>> },
    { D3DRS_BLENDOP, HostToGuestFunction<SetRenderState<D3DRS_BLENDOP>> },
    { D3DRS_SCISSORTESTENABLE, HostToGuestFunction<SetRenderState<D3DRS_SCISSORTESTENABLE>> },
    { D3DRS_SLOPESCALEDEPTHBIAS, HostToGuestFunction<SetRenderState<D3DRS_SLOPESCALEDEPTHBIAS>> },
    { D3DRS_DEPTHBIAS, HostToGuestFunction<SetRenderState<D3DRS_DEPTHBIAS>> },
    { D3DRS_SRCBLENDALPHA, HostToGuestFunction<SetRenderState<D3DRS_SRCBLENDALPHA>> },
    { D3DRS_DESTBLENDALPHA, HostToGuestFunction<SetRenderState<D3DRS_DESTBLENDALPHA>> },
    { D3DRS_BLENDOPALPHA, HostToGuestFunction<SetRenderState<D3DRS_BLENDOPALPHA>> },
    { D3DRS_COLORWRITEENABLE, HostToGuestFunction<SetRenderState<D3DRS_COLORWRITEENABLE>> }
};

static std::unique_ptr<RenderPipeline> g_resolveMsaaDepthPipelines[3];

enum
{
    GAUSSIAN_BLUR_3X3,
    GAUSSIAN_BLUR_5X5,
    GAUSSIAN_BLUR_7X7,
    GAUSSIAN_BLUR_9X9,
    GAUSSIAN_BLUR_COUNT
};

static std::unique_ptr<GuestShader> g_gaussianBlurShaders[GAUSSIAN_BLUR_COUNT];

#define CREATE_SHADER(NAME) \
    g_device->createShader( \
        g_vulkan ? g_##NAME##_spirv : g_##NAME##_dxil, \
        g_vulkan ? sizeof(g_##NAME##_spirv) : sizeof(g_##NAME##_dxil), \
        "main", \
        g_vulkan ? RenderShaderFormat::SPIRV : RenderShaderFormat::DXIL)

static bool DetectWine()
{
    HMODULE dllHandle = GetModuleHandle("ntdll.dll");
    return dllHandle != nullptr && GetProcAddress(dllHandle, "wine_get_version") != nullptr;
}

static constexpr size_t TEXTURE_DESCRIPTOR_SIZE = 65536;
static constexpr size_t SAMPLER_DESCRIPTOR_SIZE = 1024;

static std::unique_ptr<GuestTexture> g_imFontTexture;
static std::unique_ptr<RenderPipelineLayout> g_imPipelineLayout;
static std::unique_ptr<RenderPipeline> g_imPipeline;
static ImDrawDataSnapshot g_imSnapshot;

template<typename T>
static void ExecuteCopyCommandList(const T& function)
{
    std::lock_guard lock(g_copyMutex);

    g_copyCommandList->begin();
    function();
    g_copyCommandList->end();
    g_copyQueue->executeCommandLists(g_copyCommandList.get(), g_copyCommandFence.get());
    g_copyQueue->waitForCommandFence(g_copyCommandFence.get());
}

static constexpr uint32_t PITCH_ALIGNMENT = 0x100;
static constexpr uint32_t PLACEMENT_ALIGNMENT = 0x200;

struct ImGuiPushConstants
{
    ImVec2 gradientMin{};
    ImVec2 gradientMax{};
    ImU32 gradientTop{};
    ImU32 gradientBottom{};
    uint32_t shaderModifier{};
    uint32_t texture2DDescriptorIndex{};
    ImVec2 inverseDisplaySize{};
    ImVec2 origin{ 0.0f, 0.0f };
    ImVec2 scale{ 1.0f, 1.0f };
};

static void CreateImGuiBackend()
{
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

#ifdef ENABLE_IM_FONT_ATLAS_SNAPSHOT
    IM_DELETE(io.Fonts);
    io.Fonts = ImFontAtlasSnapshot::Load();
#else
    io.Fonts->TexDesiredWidth = 4096;
    io.Fonts->AddFontDefault();
    ImFontAtlasSnapshot::GenerateGlyphRanges();
#endif

    AchievementMenu::Init();
    AchievementOverlay::Init();
    ButtonGuide::Init();
    MessageWindow::Init();
    OptionsMenu::Init();
    InstallerWizard::Init();

    ImGui_ImplSDL2_InitForOther(Window::s_pWindow);

    RenderComponentMapping componentMapping(RenderSwizzle::ONE, RenderSwizzle::ONE, RenderSwizzle::ONE, RenderSwizzle::R);

#ifdef ENABLE_IM_FONT_ATLAS_SNAPSHOT
    g_imFontTexture = LoadTexture(decompressZstd(g_im_font_atlas_texture, g_im_font_atlas_texture_uncompressed_size).get(),
        g_im_font_atlas_texture_uncompressed_size, componentMapping);
#else
    g_imFontTexture = std::make_unique<GuestTexture>(ResourceType::Texture);

    uint8_t* pixels;
    int width, height;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    RenderTextureDesc textureDesc;
    textureDesc.dimension = RenderTextureDimension::TEXTURE_2D;
    textureDesc.width = width;
    textureDesc.height = height;
    textureDesc.depth = 1;
    textureDesc.mipLevels = 1;
    textureDesc.arraySize = 1;
    textureDesc.format = RenderFormat::R8_UNORM;

    g_imFontTexture->textureHolder = g_device->createTexture(textureDesc);
    g_imFontTexture->texture = g_imFontTexture->textureHolder.get();

    uint32_t rowPitch = (width + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
    uint32_t slicePitch = (rowPitch * height + PLACEMENT_ALIGNMENT - 1) & ~(PLACEMENT_ALIGNMENT - 1);
    auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(slicePitch));
    uint8_t* mappedMemory = reinterpret_cast<uint8_t*>(uploadBuffer->map());

    if (rowPitch == width)
    {
        memcpy(mappedMemory, pixels, slicePitch);
    }
    else
    {
        for (size_t i = 0; i < height; i++)
        {
            memcpy(mappedMemory, pixels, width);
            pixels += width;
            mappedMemory += rowPitch;
        }
    }

    uploadBuffer->unmap();

    ExecuteCopyCommandList([&]
        {
            g_copyCommandList->barriers(RenderBarrierStage::COPY, RenderTextureBarrier(g_imFontTexture->texture, RenderTextureLayout::COPY_DEST));

            g_copyCommandList->copyTextureRegion(
                RenderTextureCopyLocation::Subresource(g_imFontTexture->texture, 0),
                RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), RenderFormat::R8_UNORM, width, height, 1, rowPitch, 0));
        });

    g_imFontTexture->layout = RenderTextureLayout::COPY_DEST;

    RenderTextureViewDesc textureViewDesc;
    textureViewDesc.format = textureDesc.format;
    textureViewDesc.dimension = RenderTextureViewDimension::TEXTURE_2D;
    textureViewDesc.mipLevels = 1;
    textureViewDesc.componentMapping = componentMapping;
    g_imFontTexture->textureView = g_imFontTexture->texture->createTextureView(textureViewDesc);

    g_imFontTexture->descriptorIndex = g_textureDescriptorAllocator.allocate();
    g_textureDescriptorSet->setTexture(g_imFontTexture->descriptorIndex, g_imFontTexture->texture, RenderTextureLayout::SHADER_READ, g_imFontTexture->textureView.get());
#endif

    io.Fonts->SetTexID(g_imFontTexture.get());

    RenderPipelineLayoutBuilder pipelineLayoutBuilder;
    pipelineLayoutBuilder.begin(false, true);

    RenderDescriptorSetBuilder descriptorSetBuilder;
    descriptorSetBuilder.begin();
    descriptorSetBuilder.addTexture(0, TEXTURE_DESCRIPTOR_SIZE);
    descriptorSetBuilder.end(true, TEXTURE_DESCRIPTOR_SIZE);
    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);

    descriptorSetBuilder.begin();
    descriptorSetBuilder.addSampler(0, SAMPLER_DESCRIPTOR_SIZE);
    descriptorSetBuilder.end(true, SAMPLER_DESCRIPTOR_SIZE);
    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);

    pipelineLayoutBuilder.addPushConstant(0, 2, sizeof(ImGuiPushConstants), RenderShaderStageFlag::VERTEX | RenderShaderStageFlag::PIXEL);

    pipelineLayoutBuilder.end();
    g_imPipelineLayout = pipelineLayoutBuilder.create(g_device.get());

    auto vertexShader = CREATE_SHADER(imgui_vs);
    auto pixelShader = CREATE_SHADER(imgui_ps);

    RenderInputElement inputElements[3];
    inputElements[0] = RenderInputElement("POSITION", 0, 0, RenderFormat::R32G32_FLOAT, 0, offsetof(ImDrawVert, pos));
    inputElements[1] = RenderInputElement("TEXCOORD", 0, 1, RenderFormat::R32G32_FLOAT, 0, offsetof(ImDrawVert, uv));
    inputElements[2] = RenderInputElement("COLOR", 0, 2, RenderFormat::R8G8B8A8_UNORM, 0, offsetof(ImDrawVert, col));

    RenderInputSlot inputSlot(0, sizeof(ImDrawVert));

    RenderGraphicsPipelineDesc pipelineDesc;
    pipelineDesc.pipelineLayout = g_imPipelineLayout.get();
    pipelineDesc.vertexShader = vertexShader.get();
    pipelineDesc.pixelShader = pixelShader.get();
    pipelineDesc.renderTargetFormat[0] = BACKBUFFER_FORMAT;
    pipelineDesc.renderTargetBlend[0] = RenderBlendDesc::AlphaBlend();
    pipelineDesc.renderTargetCount = 1;
    pipelineDesc.inputElements = inputElements;
    pipelineDesc.inputElementsCount = std::size(inputElements);
    pipelineDesc.inputSlots = &inputSlot;
    pipelineDesc.inputSlotsCount = 1;
    g_imPipeline = g_device->createGraphicsPipeline(pipelineDesc);

#ifndef ENABLE_IM_FONT_ATLAS_SNAPSHOT
    ImFontAtlasSnapshot snapshot;
    snapshot.Snap();

    FILE* file = fopen("im_font_atlas.bin", "wb");
    if (file)
    {
        fwrite(snapshot.data.data(), 1, snapshot.data.size(), file);
        fclose(file);
    }

    ddspp::Header header;
    ddspp::HeaderDXT10 headerDX10;
    ddspp::encode_header(ddspp::R8_UNORM, width, height, 1, ddspp::Texture2D, 1, 1, header, headerDX10);

    file = fopen("im_font_atlas.dds", "wb");
    if (file)
    {
        fwrite(&ddspp::DDS_MAGIC, 4, 1, file);
        fwrite(&header, sizeof(header), 1, file);
        fwrite(&headerDX10, sizeof(headerDX10), 1, file);
        fwrite(pixels, 1, width * height, file);
        fclose(file);
    }
#endif
}

static void BeginCommandList();

void Video::CreateHostDevice()
{
    for (uint32_t i = 0; i < 16; i++)
        g_inputSlots[i].index = i;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    Window::Init();

    g_vulkan = DetectWine() || Config::GraphicsAPI == EGraphicsAPI::Vulkan;

    LoadEmbeddedResources();

    g_interface = g_vulkan ? CreateVulkanInterface() : CreateD3D12Interface();
    g_device = g_interface->createDevice();

    g_triangleFanSupported = g_device->getCapabilities().triangleFan;

    g_queue = g_device->createCommandQueue(RenderCommandListType::DIRECT);

    for (auto& commandList : g_commandLists)
        commandList = g_device->createCommandList(RenderCommandListType::DIRECT);

    for (auto& commandFence : g_commandFences)
        commandFence = g_device->createCommandFence();

    g_copyQueue = g_device->createCommandQueue(RenderCommandListType::COPY);
    g_copyCommandList = g_device->createCommandList(RenderCommandListType::COPY);
    g_copyCommandFence = g_device->createCommandFence();

    uint32_t bufferCount = 2;

    switch (Config::TripleBuffering)
    {
    case ETripleBuffering::Auto:
        bufferCount = g_vulkan ? 2 : 3; // Defaulting to 3 is fine on D3D12 thanks to flip discard model.
        break;
    case ETripleBuffering::On:
        bufferCount = 3;
        break;
    case ETripleBuffering::Off:
        bufferCount = 2;
        break;
    }

    g_swapChain = g_queue->createSwapChain(Window::s_handle, bufferCount, BACKBUFFER_FORMAT);
    g_swapChain->setVsyncEnabled(Config::VSync);
    g_swapChainValid = !g_swapChain->needsResize();

    for (auto& acquireSemaphore : g_acquireSemaphores)
        acquireSemaphore = g_device->createCommandSemaphore();
    
    for (auto& renderSemaphore : g_renderSemaphores)
        renderSemaphore = g_device->createCommandSemaphore();

    g_mainThreadId = GetCurrentThreadId();

    RenderPipelineLayoutBuilder pipelineLayoutBuilder;
    pipelineLayoutBuilder.begin(false, true);
    
    RenderDescriptorSetBuilder descriptorSetBuilder;
    descriptorSetBuilder.begin();
    descriptorSetBuilder.addTexture(0, TEXTURE_DESCRIPTOR_SIZE);
    descriptorSetBuilder.end(true, TEXTURE_DESCRIPTOR_SIZE);
    
    g_textureDescriptorSet = descriptorSetBuilder.create(g_device.get());
    
    for (size_t i = 0; i < TEXTURE_DESCRIPTOR_NULL_COUNT; i++)
    {
        auto& texture = g_blankTextures[i];
        auto& textureView = g_blankTextureViews[i];

        RenderTextureDesc desc;
        desc.width = 1;
        desc.height = 1;
        desc.depth = 1;
        desc.mipLevels = 1;
        desc.format = RenderFormat::R8_UNORM;

        RenderTextureViewDesc viewDesc;
        viewDesc.format = desc.format;
        viewDesc.componentMapping = RenderComponentMapping(RenderSwizzle::ZERO, RenderSwizzle::ZERO, RenderSwizzle::ZERO, RenderSwizzle::ZERO);
        viewDesc.mipLevels = 1;

        switch (i)
        {
        case TEXTURE_DESCRIPTOR_NULL_TEXTURE_2D:
            desc.dimension = RenderTextureDimension::TEXTURE_2D;
            desc.arraySize = 1;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_2D;
            break;

        case TEXTURE_DESCRIPTOR_NULL_TEXTURE_3D:
            desc.dimension = RenderTextureDimension::TEXTURE_3D;
            desc.arraySize = 1;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_3D;
            break;

        case TEXTURE_DESCRIPTOR_NULL_TEXTURE_CUBE:
            desc.dimension = RenderTextureDimension::TEXTURE_2D;
            desc.arraySize = 6;
            desc.flags = RenderTextureFlag::CUBE;
            viewDesc.dimension = RenderTextureViewDimension::TEXTURE_CUBE;
            break;

        default:
            assert(false && "Unknown null descriptor dimension");
            break;
        }

        texture = g_device->createTexture(desc);
        textureView = texture->createTextureView(viewDesc);

        g_textureDescriptorSet->setTexture(i, texture.get(), RenderTextureLayout::SHADER_READ, textureView.get());
    }

    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);
    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);
    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);
    
    descriptorSetBuilder.begin();
    descriptorSetBuilder.addSampler(0, SAMPLER_DESCRIPTOR_SIZE);
    descriptorSetBuilder.end(true, SAMPLER_DESCRIPTOR_SIZE);
    
    g_samplerDescriptorSet = descriptorSetBuilder.create(g_device.get());
    auto& [descriptorIndex, sampler] = g_samplerStates[XXH3_64bits(&g_samplerDescs[0], sizeof(RenderSamplerDesc))];
    descriptorIndex = 1;
    sampler = g_device->createSampler(g_samplerDescs[0]);
    g_samplerDescriptorSet->setSampler(0, sampler.get());

    pipelineLayoutBuilder.addDescriptorSet(descriptorSetBuilder);

    if (g_vulkan)
    {
        pipelineLayoutBuilder.addPushConstant(0, 4, 24, RenderShaderStageFlag::VERTEX | RenderShaderStageFlag::PIXEL);
    }
    else
    {
        pipelineLayoutBuilder.addRootDescriptor(0, 4, RenderRootDescriptorType::CONSTANT_BUFFER);
        pipelineLayoutBuilder.addRootDescriptor(1, 4, RenderRootDescriptorType::CONSTANT_BUFFER);
        pipelineLayoutBuilder.addRootDescriptor(2, 4, RenderRootDescriptorType::CONSTANT_BUFFER);
        pipelineLayoutBuilder.addPushConstant(3, 4, 4, RenderShaderStageFlag::PIXEL); // For copy/resolve shaders.
    }
    pipelineLayoutBuilder.end();
    
    g_pipelineLayout = pipelineLayoutBuilder.create(g_device.get());

    auto copyShader = CREATE_SHADER(copy_vs);

    for (size_t i = 0; i < std::size(g_resolveMsaaDepthPipelines); i++)
    {
        std::unique_ptr<RenderShader> pixelShader;
        switch (i)
        {
        case 0:
            pixelShader = CREATE_SHADER(resolve_msaa_depth_2x);
            break;
        case 1:
            pixelShader = CREATE_SHADER(resolve_msaa_depth_4x);
            break;
        case 2:
            pixelShader = CREATE_SHADER(resolve_msaa_depth_8x);
            break;
        }

        RenderGraphicsPipelineDesc desc;
        desc.pipelineLayout = g_pipelineLayout.get();
        desc.vertexShader = copyShader.get();
        desc.pixelShader = pixelShader.get();
        desc.depthFunction = RenderComparisonFunction::ALWAYS;
        desc.depthEnabled = true;
        desc.depthWriteEnabled = true;
        desc.depthTargetFormat = RenderFormat::D32_FLOAT;
        g_resolveMsaaDepthPipelines[i] = g_device->createGraphicsPipeline(desc);
    }

    for (auto& shader : g_gaussianBlurShaders)
        shader = std::make_unique<GuestShader>(ResourceType::PixelShader);

    g_gaussianBlurShaders[GAUSSIAN_BLUR_3X3]->shader = CREATE_SHADER(gaussian_blur_3x3);
    g_gaussianBlurShaders[GAUSSIAN_BLUR_5X5]->shader = CREATE_SHADER(gaussian_blur_5x5);
    g_gaussianBlurShaders[GAUSSIAN_BLUR_7X7]->shader = CREATE_SHADER(gaussian_blur_7x7);
    g_gaussianBlurShaders[GAUSSIAN_BLUR_9X9]->shader = CREATE_SHADER(gaussian_blur_9x9);

    CreateImGuiBackend();

    auto gammaCorrectionShader = CREATE_SHADER(gamma_correction_ps);

    RenderGraphicsPipelineDesc desc;
    desc.pipelineLayout = g_pipelineLayout.get();
    desc.vertexShader = copyShader.get();
    desc.pixelShader = gammaCorrectionShader.get();
    desc.renderTargetFormat[0] = BACKBUFFER_FORMAT;
    desc.renderTargetBlend[0] = RenderBlendDesc::Copy();
    desc.renderTargetCount = 1;
    g_gammaCorrectionPipeline = g_device->createGraphicsPipeline(desc);

    g_backBuffer = g_userHeap.AllocPhysical<GuestSurface>(ResourceType::RenderTarget);
    g_backBuffer->width = 1280;
    g_backBuffer->height = 720;
    g_backBuffer->format = BACKBUFFER_FORMAT;
    g_backBuffer->textureHolder = g_device->createTexture(RenderTextureDesc::Texture2D(1, 1, 1, BACKBUFFER_FORMAT, RenderTextureFlag::RENDER_TARGET));

    BeginCommandList();

    RenderTextureBarrier blankTextureBarriers[TEXTURE_DESCRIPTOR_NULL_COUNT];
    for (size_t i = 0; i < TEXTURE_DESCRIPTOR_NULL_COUNT; i++)
        blankTextureBarriers[i] = RenderTextureBarrier(g_blankTextures[i].get(), RenderTextureLayout::SHADER_READ);

    g_commandLists[g_frame]->barriers(RenderBarrierStage::NONE, blankTextureBarriers, std::size(blankTextureBarriers));
}

void Video::WaitForGPU()
{
    if (g_vulkan)
    {
        g_device->waitIdle();
    }
    else 
    {
        for (size_t i = 0; i < NUM_FRAMES; i++)
        {
            if (g_commandListStates[i])
            {
                g_queue->waitForCommandFence(g_commandFences[i].get());
                g_commandListStates[i] = false;
            }
        }
        g_queue->executeCommandLists(nullptr, g_commandFences[0].get());
        g_queue->waitForCommandFence(g_commandFences[0].get());
    }
}

static std::atomic<bool> g_pendingRenderThread;

static void WaitForRenderThread()
{
    g_pendingRenderThread.wait(true);
}

static void BeginCommandList()
{
    g_renderTarget = g_backBuffer;
    g_depthStencil = nullptr;
    g_framebuffer = nullptr;

    g_pipelineState.renderTargetFormat = BACKBUFFER_FORMAT;
    g_pipelineState.depthStencilFormat = RenderFormat::UNKNOWN;

    g_swapChain->setVsyncEnabled(Config::VSync);
    g_swapChainValid &= !g_swapChain->needsResize();

    if (!g_swapChainValid)
    {
        Video::WaitForGPU();
        g_backBuffer->framebuffers.clear();
        g_swapChainValid = g_swapChain->resize();
        g_needsResize = g_swapChainValid;
    }

    if (g_swapChainValid)
        g_swapChainValid = g_swapChain->acquireTexture(g_acquireSemaphores[g_frame].get(), &g_backBufferIndex);

    if (g_swapChainValid)
    {
        bool applyingGammaCorrection = Config::XboxColourCorrection || abs(Config::Brightness - 0.5f) > 0.001f;

        if (applyingGammaCorrection)
        {
            uint32_t width = g_swapChain->getWidth();
            uint32_t height = g_swapChain->getHeight();

            if (g_intermediaryBackBufferTextureWidth != width ||
                g_intermediaryBackBufferTextureHeight != height)
            {
                if (g_intermediaryBackBufferTextureDescriptorIndex == NULL)
                    g_intermediaryBackBufferTextureDescriptorIndex = g_textureDescriptorAllocator.allocate();

                Video::WaitForGPU(); // Fine to wait for GPU, this'll only happen during resize.

                g_intermediaryBackBufferTexture = g_device->createTexture(RenderTextureDesc::Texture2D(width, height, 1, BACKBUFFER_FORMAT, RenderTextureFlag::RENDER_TARGET));
                g_textureDescriptorSet->setTexture(g_intermediaryBackBufferTextureDescriptorIndex, g_intermediaryBackBufferTexture.get(), RenderTextureLayout::SHADER_READ);

                g_intermediaryBackBufferTextureWidth = width;
                g_intermediaryBackBufferTextureHeight = height;
            }

            g_backBuffer->texture = g_intermediaryBackBufferTexture.get();
        }
        else
        {
            g_backBuffer->texture = g_swapChain->getTexture(g_backBufferIndex);
        }
    }
    else
    {
        g_backBuffer->texture = g_backBuffer->textureHolder.get();
    }

    g_backBuffer->layout = RenderTextureLayout::UNKNOWN;

    for (size_t i = 0; i < 16; i++)
    {
        g_sharedConstants.texture2DIndices[i] = TEXTURE_DESCRIPTOR_NULL_TEXTURE_2D;
        g_sharedConstants.texture3DIndices[i] = TEXTURE_DESCRIPTOR_NULL_TEXTURE_3D;
        g_sharedConstants.textureCubeIndices[i] = TEXTURE_DESCRIPTOR_NULL_TEXTURE_CUBE;
    }

    if (Config::GITextureFiltering == EGITextureFiltering::Bicubic)
        g_pipelineState.specConstants |= SPEC_CONSTANT_BICUBIC_GI_FILTER;
    else
        g_pipelineState.specConstants &= ~SPEC_CONSTANT_BICUBIC_GI_FILTER;

    auto& commandList = g_commandLists[g_frame];

    commandList->begin();
    commandList->setGraphicsPipelineLayout(g_pipelineLayout.get());
    commandList->setGraphicsDescriptorSet(g_textureDescriptorSet.get(), 0);
    commandList->setGraphicsDescriptorSet(g_textureDescriptorSet.get(), 1);
    commandList->setGraphicsDescriptorSet(g_textureDescriptorSet.get(), 2);
    commandList->setGraphicsDescriptorSet(g_samplerDescriptorSet.get(), 3);
}

static uint32_t CreateDevice(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, be<uint32_t>* a6)
{
    g_xdbfTextureCache = std::unordered_map<uint16_t, GuestTexture *>();

    for (auto &achievement : g_xdbfWrapper.GetAchievements(XDBF_LANGUAGE_ENGLISH))
    {
        // huh?
        if (!achievement.pImageBuffer || !achievement.ImageBufferSize)
            continue;

        g_xdbfTextureCache[achievement.ID] =
            LoadTexture((uint8_t *)achievement.pImageBuffer, achievement.ImageBufferSize).release();
    }

    auto device = g_userHeap.AllocPhysical<GuestDevice>();
    memset(device, 0, sizeof(*device));

    uint32_t functionOffset = 0x443344; // D3D
    g_codeCache.Insert(functionOffset, reinterpret_cast<void*>(HostToGuestFunction<SetRenderStateUnimplemented>));

    for (size_t i = 0; i < _countof(device->setRenderStateFunctions); i++)
        device->setRenderStateFunctions[i] = functionOffset;

    for (auto& [state, function] : g_setRenderStateFunctions)
    {
        functionOffset += 4;
        g_codeCache.Insert(functionOffset, function);
        device->setRenderStateFunctions[state / 4] = functionOffset;
    }

    for (size_t i = 0; i < _countof(device->setSamplerStateFunctions); i++)
        device->setSamplerStateFunctions[i] = *reinterpret_cast<uint32_t*>(g_memory.Translate(0x8330F3DC + i * 0xC));

    device->viewport.width = 1280.0f;
    device->viewport.height = 720.0f;
    device->viewport.maxZ = 1.0f;

    *a6 = g_memory.MapVirtual(device);

    return 0;
}

static void DestructResource(GuestResource* resource) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::DestructResource;
    cmd.destructResource.resource = resource;
    g_renderQueue.enqueue(cmd);
}

static void ProcDestructResource(const RenderCommand& cmd)
{
    const auto& args = cmd.destructResource;
    g_tempResources[g_frame].push_back(args.resource);
}

static uint32_t ComputeTexturePitch(GuestTexture* texture)
{
    return (texture->width * RenderFormatSize(texture->format) + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
}

static void LockTextureRect(GuestTexture* texture, uint32_t, GuestLockedRect* lockedRect) 
{
    uint32_t pitch = ComputeTexturePitch(texture);
    uint32_t slicePitch = pitch * texture->height;

    if (texture->mappedMemory == nullptr)
        texture->mappedMemory = g_userHeap.AllocPhysical(slicePitch, 0x10);

    lockedRect->pitch = pitch;
    lockedRect->bits = g_memory.MapVirtual(texture->mappedMemory);
}

static void UnlockTextureRect(GuestTexture* texture) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::UnlockTextureRect;
    cmd.unlockTextureRect.texture = texture;
    g_renderQueue.enqueue(cmd);
}

static void ProcUnlockTextureRect(const RenderCommand& cmd)
{
    const auto& args = cmd.unlockTextureRect;

    AddBarrier(args.texture, RenderTextureLayout::COPY_DEST);
    FlushBarriers();

    uint32_t pitch = ComputeTexturePitch(args.texture);
    uint32_t slicePitch = pitch * args.texture->height;

    auto allocation = g_uploadAllocators[g_frame].allocate(slicePitch, PLACEMENT_ALIGNMENT);
    memcpy(allocation.memory, args.texture->mappedMemory, slicePitch);

    g_commandLists[g_frame]->copyTextureRegion(
        RenderTextureCopyLocation::Subresource(args.texture->texture, 0),
        RenderTextureCopyLocation::PlacedFootprint(allocation.buffer, args.texture->format, args.texture->width, args.texture->height, 1, pitch / RenderFormatSize(args.texture->format), allocation.offset));
}

static void* LockBuffer(GuestBuffer* buffer, uint32_t flags)
{
    buffer->lockedReadOnly = (flags & 0x10) != 0;

    if (buffer->mappedMemory == nullptr)
        buffer->mappedMemory = g_userHeap.AllocPhysical(buffer->dataSize, 0x10);

    return buffer->mappedMemory;
}

static void* LockVertexBuffer(GuestBuffer* buffer, uint32_t, uint32_t, uint32_t flags)
{
    return LockBuffer(buffer, flags);
}

template<typename T>
static void UnlockBuffer(GuestBuffer* buffer, bool useCopyQueue)
{
    auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(buffer->dataSize));

    auto dest = reinterpret_cast<T*>(uploadBuffer->map());
    auto src = reinterpret_cast<const T*>(buffer->mappedMemory);

    for (size_t i = 0; i < buffer->dataSize; i += sizeof(T))
    {
        *dest = std::byteswap(*src);
        ++dest;
        ++src;
    }

    uploadBuffer->unmap();

    if (useCopyQueue)
    {
        ExecuteCopyCommandList([&]
            {
                g_copyCommandList->copyBufferRegion(buffer->buffer->at(0), uploadBuffer->at(0), buffer->dataSize);
            });
    }
    else
    {
        auto& commandList = g_commandLists[g_frame];

        commandList->barriers(RenderBarrierStage::COPY, RenderBufferBarrier(buffer->buffer.get(), RenderBufferAccess::WRITE));
        commandList->copyBufferRegion(buffer->buffer->at(0), uploadBuffer->at(0), buffer->dataSize);
        commandList->barriers(RenderBarrierStage::GRAPHICS, RenderBufferBarrier(buffer->buffer.get(), RenderBufferAccess::READ));

        g_tempBuffers[g_frame].emplace_back(std::move(uploadBuffer));
    }
}

template<typename T>
static void UnlockBuffer(GuestBuffer* buffer)
{
    if (!buffer->lockedReadOnly)
    {
        if (GetCurrentThreadId() == g_mainThreadId)
        {
            RenderCommand cmd;
            cmd.type = (sizeof(T) == 2) ? RenderCommandType::UnlockBuffer16 : RenderCommandType::UnlockBuffer32;
            cmd.unlockBuffer.buffer = buffer;
            g_renderQueue.enqueue(cmd);
        }
        else
        {
            UnlockBuffer<T>(buffer, true);
        }
    }
}

static void ProcUnlockBuffer16(const RenderCommand& cmd)
{
    UnlockBuffer<uint16_t>(cmd.unlockBuffer.buffer, false);
}

static void ProcUnlockBuffer32(const RenderCommand& cmd)
{
    UnlockBuffer<uint32_t>(cmd.unlockBuffer.buffer, false);
}

static void UnlockVertexBuffer(GuestBuffer* buffer)
{
    UnlockBuffer<uint32_t>(buffer);
}

static void GetVertexBufferDesc(GuestBuffer* buffer, GuestBufferDesc* desc) 
{
    desc->size = buffer->dataSize;
}

static void* LockIndexBuffer(GuestBuffer* buffer, uint32_t, uint32_t, uint32_t flags) 
{
    return LockBuffer(buffer, flags);
}

static void UnlockIndexBuffer(GuestBuffer* buffer) 
{
    UnlockBuffer<uint16_t>(buffer);
}

static void GetIndexBufferDesc(GuestBuffer* buffer, GuestBufferDesc* desc)
{
    desc->format = buffer->guestFormat;
    desc->size = buffer->dataSize;
}

static void GetSurfaceDesc(GuestSurface* surface, GuestSurfaceDesc* desc) 
{
    desc->width = surface->width;
    desc->height = surface->height;
}

static void GetVertexDeclaration(GuestVertexDeclaration* vertexDeclaration, GuestVertexElement* vertexElements, be<uint32_t>* count) 
{
    memcpy(vertexElements, vertexDeclaration->vertexElements.get(), vertexDeclaration->vertexElementCount * sizeof(GuestVertexElement));
    *count = vertexDeclaration->vertexElementCount;
}

static uint32_t HashVertexDeclaration(uint32_t vertexDeclaration) 
{
    // Vertex declarations are cached on host side, so the pointer itself can be used.
    return vertexDeclaration;
}

static void DrawImGui()
{
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ResetImGuiCallbacks();

#ifdef ASYNC_PSO_DEBUG
    if (ImGui::Begin("Async PSO Stats"))
    {
        ImGui::Text("Pipelines Created In Render Thread: %d", g_pipelinesCreatedInRenderThread.load());
        ImGui::Text("Pipelines Created Asynchronously: %d", g_pipelinesCreatedAsynchronously.load());
        ImGui::Text("Pipelines Dropped: %d", g_pipelinesDropped.load());
        ImGui::Text("Pipelines Currently Compiling: %d", g_pipelinesCurrentlyCompiling.load());
        ImGui::Text("Compiling Data Count: %d", g_compilingDataCount.load());
        ImGui::Text("Pending Data Count: %d", g_pendingDataCount.load());

        std::lock_guard lock(g_debugMutex);
        ImGui::TextUnformatted(g_pipelineDebugText.c_str());
    }
    ImGui::End();
#endif

    AchievementMenu::Draw();
    OptionsMenu::Draw();
    AchievementOverlay::Draw();
    InstallerWizard::Draw();
    Fader::Draw();
    MessageWindow::Draw();
    ButtonGuide::Draw();

    ImGui::Render();

    auto drawData = ImGui::GetDrawData();
    if (drawData->CmdListsCount != 0)
    {
        g_imSnapshot.SnapUsingSwap(drawData, ImGui::GetTime());

        RenderCommand cmd;
        cmd.type = RenderCommandType::DrawImGui;
        g_renderQueue.enqueue(cmd);
    }
}

static void SetFramebuffer(GuestSurface *renderTarget, GuestSurface *depthStencil, bool settingForClear);

static void ProcDrawImGui(const RenderCommand& cmd)
{
    // Make sure the backbuffer is the current target.
    AddBarrier(g_backBuffer, RenderTextureLayout::COLOR_WRITE);
    FlushBarriers();
    SetFramebuffer(g_backBuffer, nullptr, false);

    auto& commandList = g_commandLists[g_frame];

    commandList->setGraphicsPipelineLayout(g_imPipelineLayout.get());
    commandList->setPipeline(g_imPipeline.get());
    commandList->setGraphicsDescriptorSet(g_textureDescriptorSet.get(), 0);
    commandList->setGraphicsDescriptorSet(g_samplerDescriptorSet.get(), 1);

    auto& drawData = g_imSnapshot.DrawData;
    commandList->setViewports(RenderViewport(drawData.DisplayPos.x, drawData.DisplayPos.y, drawData.DisplaySize.x, drawData.DisplaySize.y));

    ImGuiPushConstants pushConstants{};
    pushConstants.inverseDisplaySize = { 1.0f / drawData.DisplaySize.x, 1.0f / drawData.DisplaySize.y };
    commandList->setGraphicsPushConstants(0, &pushConstants);

    for (int i = 0; i < drawData.CmdListsCount; i++)
    {
        auto& drawList = drawData.CmdLists[i];

        auto vertexBufferAllocation = g_uploadAllocators[g_frame].allocate<false>(drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert), alignof(ImDrawVert));
        auto indexBufferAllocation = g_uploadAllocators[g_frame].allocate<false>(drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(uint16_t), alignof(uint16_t));

        const RenderVertexBufferView vertexBufferView(vertexBufferAllocation.buffer->at(vertexBufferAllocation.offset), drawList->VtxBuffer.Size * sizeof(ImDrawVert));
        const RenderInputSlot inputSlot(0, sizeof(ImDrawVert));
        commandList->setVertexBuffers(0, &vertexBufferView, 1, &inputSlot);

        const RenderIndexBufferView indexBufferView(indexBufferAllocation.buffer->at(indexBufferAllocation.offset), drawList->IdxBuffer.Size * sizeof(uint16_t), RenderFormat::R16_UINT);
        commandList->setIndexBuffer(&indexBufferView);

        for (int j = 0; j < drawList->CmdBuffer.Size; j++)
        {
            auto& drawCmd = drawList->CmdBuffer[j];
            if (drawCmd.UserCallback != nullptr)
            {
                auto callbackData = reinterpret_cast<const ImGuiCallbackData*>(drawCmd.UserCallbackData);

                switch (static_cast<ImGuiCallback>(reinterpret_cast<size_t>(drawCmd.UserCallback)))
                {
                case ImGuiCallback::SetGradient:
                    commandList->setGraphicsPushConstants(0, &callbackData->setGradient, offsetof(ImGuiPushConstants, gradientMin), sizeof(callbackData->setGradient));
                    break;       
                case ImGuiCallback::SetShaderModifier:
                    commandList->setGraphicsPushConstants(0, &callbackData->setShaderModifier, offsetof(ImGuiPushConstants, shaderModifier), sizeof(callbackData->setShaderModifier));
                    break;
                case ImGuiCallback::SetOrigin:
                    commandList->setGraphicsPushConstants(0, &callbackData->setOrigin, offsetof(ImGuiPushConstants, origin), sizeof(callbackData->setOrigin));
                    break;
                case ImGuiCallback::SetScale:
                    commandList->setGraphicsPushConstants(0, &callbackData->setScale, offsetof(ImGuiPushConstants, scale), sizeof(callbackData->setScale));
                    break;
                }
            }
            else
            {
                if (drawCmd.ClipRect.z <= drawCmd.ClipRect.x || drawCmd.ClipRect.w <= drawCmd.ClipRect.y)
                    continue;

                auto texture = reinterpret_cast<GuestTexture*>(drawCmd.TextureId);
                uint32_t descriptorIndex = TEXTURE_DESCRIPTOR_NULL_TEXTURE_2D;
                if (texture != nullptr)
                {
                    if (texture->layout != RenderTextureLayout::SHADER_READ)
                    {
                        commandList->barriers(RenderBarrierStage::GRAPHICS | RenderBarrierStage::COPY,
                            RenderTextureBarrier(texture->texture, RenderTextureLayout::SHADER_READ));

                        texture->layout = RenderTextureLayout::SHADER_READ;
                    }

                    descriptorIndex = texture->descriptorIndex;
                }

                commandList->setGraphicsPushConstants(0, &descriptorIndex, offsetof(ImGuiPushConstants, texture2DDescriptorIndex), sizeof(descriptorIndex));
                commandList->setScissors(RenderRect(int32_t(drawCmd.ClipRect.x), int32_t(drawCmd.ClipRect.y), int32_t(drawCmd.ClipRect.z), int32_t(drawCmd.ClipRect.w)));
                commandList->drawIndexedInstanced(drawCmd.ElemCount, 1, drawCmd.IdxOffset, drawCmd.VtxOffset, 0);
            }
        }
    }
}

static bool g_shouldPrecompilePipelines = false;

void Video::HostPresent() 
{
    WaitForRenderThread();
    DrawImGui();

    g_pendingRenderThread.store(true);

    RenderCommand cmd;
    cmd.type = RenderCommandType::Present;
    g_renderQueue.enqueue(cmd);

    // All the shaders are available at this point. We can precompile embedded PSOs then.
    if (g_shouldPrecompilePipelines)
    {
        // This is all the model consumer thread needs to see.
        ++g_compilingDataCount;

        if ((++g_pendingDataCount) == 1)
            g_pendingDataCount.notify_all();

        g_shouldPrecompilePipelines = false;
    }
}

void Video::StartPipelinePrecompilation()
{
    g_shouldPrecompilePipelines = true;
}

static void GuestPresent() 
{
    Video::HostPresent();
}

static void SetRootDescriptor(const UploadAllocation& allocation, size_t index)
{
    auto& commandList = g_commandLists[g_frame];

    if (g_vulkan)
        commandList->setGraphicsPushConstants(0, &allocation.deviceAddress, 8 * index, 8);
    else
        commandList->setGraphicsRootDescriptor(allocation.buffer->at(allocation.offset), index);
}

static void ProcPresent(const RenderCommand& cmd)
{
    if (g_swapChainValid)
    {
        auto swapChainTexture = g_swapChain->getTexture(g_backBufferIndex);
        if (g_backBuffer->texture == g_intermediaryBackBufferTexture.get())
        {
            struct
            {
                float gammaR;
                float gammaG;
                float gammaB;
                uint32_t textureDescriptorIndex;
            } constants;

            if (Config::XboxColourCorrection)
            {
                constants.gammaR = 1.2f;
                constants.gammaG = 1.17f;
                constants.gammaB = 0.98f;
            }
            else
            {
                constants.gammaR = 1.0f;
                constants.gammaG = 1.0f;
                constants.gammaB = 1.0f;
            }

            float offset = (Config::Brightness - 0.5f) * 1.2f;

            constants.gammaR = 1.0f / std::clamp(constants.gammaR + offset, 0.1f, 4.0f);
            constants.gammaG = 1.0f / std::clamp(constants.gammaG + offset, 0.1f, 4.0f);
            constants.gammaB = 1.0f / std::clamp(constants.gammaB + offset, 0.1f, 4.0f);
            constants.textureDescriptorIndex = g_intermediaryBackBufferTextureDescriptorIndex;

            auto &framebuffer = g_backBuffer->framebuffers[swapChainTexture];
            if (!framebuffer)
            {
                RenderFramebufferDesc desc;
                desc.colorAttachments = const_cast<const RenderTexture **>(&swapChainTexture);
                desc.colorAttachmentsCount = 1;
                framebuffer = g_device->createFramebuffer(desc);
            }

            RenderTextureBarrier srcBarriers[] =
            {
                RenderTextureBarrier(g_intermediaryBackBufferTexture.get(), RenderTextureLayout::SHADER_READ),
                RenderTextureBarrier(swapChainTexture, RenderTextureLayout::COLOR_WRITE)
            };

            auto &commandList = g_commandLists[g_frame];
            commandList->barriers(RenderBarrierStage::GRAPHICS, srcBarriers, std::size(srcBarriers));
            commandList->setGraphicsPipelineLayout(g_pipelineLayout.get());
            commandList->setPipeline(g_gammaCorrectionPipeline.get());
            commandList->setGraphicsDescriptorSet(g_textureDescriptorSet.get(), 0);
            SetRootDescriptor(g_uploadAllocators[g_frame].allocate<false>(&constants, sizeof(constants), 0x100), 2);
            commandList->setFramebuffer(framebuffer.get());
            commandList->setViewports(RenderViewport(0.0f, 0.0f, g_intermediaryBackBufferTextureWidth, g_intermediaryBackBufferTextureHeight));
            commandList->setScissors(RenderRect(0, 0, g_intermediaryBackBufferTextureWidth, g_intermediaryBackBufferTextureHeight));
            commandList->drawInstanced(6, 1, 0, 0);
            commandList->barriers(RenderBarrierStage::GRAPHICS, RenderTextureBarrier(swapChainTexture, RenderTextureLayout::PRESENT));
        }
        else
        {
            AddBarrier(g_backBuffer, RenderTextureLayout::PRESENT);
            FlushBarriers();
        }
    }

    auto &commandList = g_commandLists[g_frame];
    commandList->end();

    if (g_swapChainValid)
    {
        const RenderCommandList *commandLists[] = { commandList.get() };
        RenderCommandSemaphore *waitSemaphores[] = { g_acquireSemaphores[g_frame].get() };
        RenderCommandSemaphore *signalSemaphores[] = { g_renderSemaphores[g_frame].get() };

        g_queue->executeCommandLists(
            commandLists, std::size(commandLists),
            waitSemaphores, std::size(waitSemaphores),
            signalSemaphores, std::size(signalSemaphores),
            g_commandFences[g_frame].get());

        g_swapChainValid = g_swapChain->present(g_backBufferIndex, signalSemaphores, std::size(signalSemaphores));
    }
    else
    {
        g_queue->executeCommandLists(commandList.get(), g_commandFences[g_frame].get());
    }

    g_commandListStates[g_frame] = true;

    g_frame = g_nextFrame;
    g_nextFrame = (g_frame + 1) % NUM_FRAMES;

    if (g_commandListStates[g_frame])
    {
        g_queue->waitForCommandFence(g_commandFences[g_frame].get());
        g_commandListStates[g_frame] = false;
    }

    g_dirtyStates = DirtyStates(true);
    g_uploadAllocators[g_frame].reset();
    DestructTempResources();
    g_triangleFanIndexData.reset();
    g_quadIndexData.reset();

    BeginCommandList();

    g_pendingRenderThread.store(false);
    g_pendingRenderThread.notify_all();
}

static GuestSurface* GetBackBuffer() 
{
    g_backBuffer->AddRef();
    return g_backBuffer;
}

static RenderFormat ConvertFormat(uint32_t format)
{
    switch (format)
    {
    case D3DFMT_A16B16G16R16F:
    case D3DFMT_A16B16G16R16F_2:
        return RenderFormat::R16G16B16A16_FLOAT;
    case D3DFMT_A8B8G8R8:
    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
        return RenderFormat::R8G8B8A8_UNORM;
    case D3DFMT_D24FS8:
    case D3DFMT_D24S8:
        return RenderFormat::D32_FLOAT;
    case D3DFMT_G16R16F:
    case D3DFMT_G16R16F_2:
        return RenderFormat::R16G16_FLOAT;
    case D3DFMT_INDEX16:
        return RenderFormat::R16_UINT;
    case D3DFMT_INDEX32:
        return RenderFormat::R32_UINT;
    case D3DFMT_L8:
    case D3DFMT_L8_2:
        return RenderFormat::R8_UNORM;
    default:
        assert(false && "Unknown format");
        return RenderFormat::R16G16B16A16_FLOAT;
    }
}

static GuestTexture* CreateTexture(uint32_t width, uint32_t height, uint32_t depth, uint32_t levels, uint32_t usage, uint32_t format, uint32_t pool, uint32_t type) 
{
    const auto texture = g_userHeap.AllocPhysical<GuestTexture>(type == 17 ? ResourceType::VolumeTexture : ResourceType::Texture);

    RenderTextureDesc desc;
    desc.dimension = texture->type == ResourceType::VolumeTexture ? RenderTextureDimension::TEXTURE_3D : RenderTextureDimension::TEXTURE_2D;
    desc.width = width;
    desc.height = height;
    desc.depth = depth;
    desc.mipLevels = levels;
    desc.arraySize = 1;
    desc.format = ConvertFormat(format);
    desc.flags = (desc.format == RenderFormat::D32_FLOAT) ? RenderTextureFlag::DEPTH_TARGET : RenderTextureFlag::NONE;

    texture->textureHolder = g_device->createTexture(desc);
    texture->texture = texture->textureHolder.get();

    RenderTextureViewDesc viewDesc;
    viewDesc.format = desc.format;
    viewDesc.dimension = texture->type == ResourceType::VolumeTexture ? RenderTextureViewDimension::TEXTURE_3D : RenderTextureViewDimension::TEXTURE_2D;
    viewDesc.mipLevels = levels;

    switch (format)
    {
    case D3DFMT_D24FS8:
    case D3DFMT_D24S8:
    case D3DFMT_L8:
    case D3DFMT_L8_2:
        viewDesc.componentMapping = RenderComponentMapping(RenderSwizzle::R, RenderSwizzle::R, RenderSwizzle::R, RenderSwizzle::ONE);
        break;

    case D3DFMT_X8R8G8B8:
        viewDesc.componentMapping = RenderComponentMapping(RenderSwizzle::G, RenderSwizzle::B, RenderSwizzle::A, RenderSwizzle::ONE);
        break;
    }

    texture->textureView = texture->texture->createTextureView(viewDesc);

    texture->width = width;
    texture->height = height;
    texture->depth = depth;
    texture->format = desc.format;
    texture->viewDimension = viewDesc.dimension;
    texture->descriptorIndex = g_textureDescriptorAllocator.allocate();

    g_textureDescriptorSet->setTexture(texture->descriptorIndex, texture->texture, RenderTextureLayout::SHADER_READ, texture->textureView.get());
   
#ifdef _DEBUG 
    texture->texture->setName(std::format("Texture {:X}", g_memory.MapVirtual(texture)));
#endif

    return texture;
}

static GuestBuffer* CreateVertexBuffer(uint32_t length) 
{
    auto buffer = g_userHeap.AllocPhysical<GuestBuffer>(ResourceType::VertexBuffer);
    buffer->buffer = g_device->createBuffer(RenderBufferDesc::VertexBuffer(length, RenderHeapType::DEFAULT, RenderBufferFlag::INDEX));
    buffer->dataSize = length;
#ifdef _DEBUG 
    buffer->buffer->setName(std::format("Vertex Buffer {:X}", g_memory.MapVirtual(buffer)));
#endif
    return buffer;
}

static GuestBuffer* CreateIndexBuffer(uint32_t length, uint32_t, uint32_t format)
{
    auto buffer = g_userHeap.AllocPhysical<GuestBuffer>(ResourceType::IndexBuffer);
    buffer->buffer = g_device->createBuffer(RenderBufferDesc::IndexBuffer(length, RenderHeapType::DEFAULT));
    buffer->dataSize = length;
    buffer->format = ConvertFormat(format);
    buffer->guestFormat = format;
#ifdef _DEBUG 
    buffer->buffer->setName(std::format("Index Buffer {:X}", g_memory.MapVirtual(buffer)));
#endif
    return buffer;
}

static GuestSurface* CreateSurface(uint32_t width, uint32_t height, uint32_t format, uint32_t multiSample) 
{
    RenderTextureDesc desc;
    desc.dimension = RenderTextureDimension::TEXTURE_2D;
    desc.width = width;
    desc.height = height;
    desc.depth = 1;
    desc.mipLevels = 1;
    desc.arraySize = 1;
    desc.multisampling.sampleCount = multiSample != 0 && Config::AntiAliasing != EAntiAliasing::None ? int32_t(Config::AntiAliasing.Value) : RenderSampleCount::COUNT_1;
    desc.format = ConvertFormat(format);
    desc.flags = desc.format == RenderFormat::D32_FLOAT ? RenderTextureFlag::DEPTH_TARGET : RenderTextureFlag::RENDER_TARGET;

    auto surface = g_userHeap.AllocPhysical<GuestSurface>(desc.format == RenderFormat::D32_FLOAT ? 
        ResourceType::DepthStencil : ResourceType::RenderTarget);

    surface->textureHolder = g_device->createTexture(desc);
    surface->texture = surface->textureHolder.get();
    surface->width = width;
    surface->height = height;
    surface->format = desc.format;
    surface->guestFormat = format;
    surface->sampleCount = desc.multisampling.sampleCount;

    if (desc.multisampling.sampleCount != RenderSampleCount::COUNT_1 && desc.format == RenderFormat::D32_FLOAT)
    {
        RenderTextureViewDesc viewDesc;
        viewDesc.dimension = RenderTextureViewDimension::TEXTURE_2D;
        viewDesc.format = RenderFormat::D32_FLOAT;
        viewDesc.mipLevels = 1;
        surface->textureView = surface->textureHolder->createTextureView(viewDesc);
        surface->descriptorIndex = g_textureDescriptorAllocator.allocate();
        g_textureDescriptorSet->setTexture(surface->descriptorIndex, surface->textureHolder.get(), RenderTextureLayout::SHADER_READ, surface->textureView.get());
    }

#ifdef _DEBUG 
    surface->texture->setName(std::format("{} {:X}", desc.flags & RenderTextureFlag::RENDER_TARGET ? "Render Target" : "Depth Stencil", g_memory.MapVirtual(surface)));
#endif

    return surface;
}

static void FlushViewport()
{
    bool renderingToBackBuffer = g_renderTarget == g_backBuffer &&
        g_backBuffer->texture != g_backBuffer->textureHolder.get();

    auto& commandList = g_commandLists[g_frame];

    if (g_dirtyStates.viewport)
    {
        auto viewport = g_viewport;
        if (g_halfPixel)
        {
            viewport.x += 0.5f;
            viewport.y += 0.5f;
        }

        if (renderingToBackBuffer)
        {
            uint32_t width = g_swapChain->getWidth();
            uint32_t height = g_swapChain->getHeight();

            viewport.x *= width / 1280.0f;
            viewport.y *= height / 720.0f;    
            viewport.width *= width / 1280.0f;
            viewport.height *= height / 720.0f;
        }

        if (viewport.minDepth > viewport.maxDepth)
            std::swap(viewport.minDepth, viewport.maxDepth);

        commandList->setViewports(viewport);

        g_dirtyStates.viewport = false;
    }

    if (g_dirtyStates.scissorRect)
    {
        auto scissorRect = g_scissorTestEnable ? g_scissorRect : RenderRect(
            g_viewport.x,
            g_viewport.y,
            g_viewport.x + g_viewport.width,
            g_viewport.y + g_viewport.height);

        if (renderingToBackBuffer)
        {
            uint32_t width = g_swapChain->getWidth();
            uint32_t height = g_swapChain->getHeight();

            scissorRect.left = scissorRect.left * width / 1280;
            scissorRect.top = scissorRect.top * height / 720;
            scissorRect.right = scissorRect.right * width / 1280;
            scissorRect.bottom = scissorRect.bottom * height / 720;
        }

        commandList->setScissors(scissorRect);

        g_dirtyStates.scissorRect = false;
    }
}

static bool SetHalfPixel(bool enable)
{
    bool oldValue = g_halfPixel;
    SetDirtyValue(g_dirtyStates.viewport, g_halfPixel, enable);
    return oldValue;
}

static void StretchRect(GuestDevice* device, uint32_t flags, uint32_t, GuestTexture* texture)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::StretchRect;
    cmd.stretchRect.flags = flags;
    cmd.stretchRect.texture = texture;
    g_renderQueue.enqueue(cmd);
}

static void ProcStretchRect(const RenderCommand& cmd)
{
    const auto& args = cmd.stretchRect;

    const bool isDepthStencil = (args.flags & 0x4) != 0;
    const auto surface = isDepthStencil ? g_depthStencil : g_renderTarget;
    const bool multiSampling = surface->sampleCount != RenderSampleCount::COUNT_1;

    RenderTextureLayout srcLayout;
    RenderTextureLayout dstLayout;

    if (multiSampling)
    {
        if (isDepthStencil)
        {
            srcLayout = RenderTextureLayout::SHADER_READ;
            dstLayout = RenderTextureLayout::DEPTH_WRITE;
        }
        else
        {
            srcLayout = RenderTextureLayout::RESOLVE_SOURCE;
            dstLayout = RenderTextureLayout::RESOLVE_DEST;
        }
    }
    else
    {
        srcLayout = RenderTextureLayout::COPY_SOURCE;
        dstLayout = RenderTextureLayout::COPY_DEST;
    }

    AddBarrier(surface, srcLayout);
    AddBarrier(args.texture, dstLayout);
    FlushBarriers();

    auto& commandList = g_commandLists[g_frame];
    if (multiSampling)
    {
        if (isDepthStencil)
        {
            uint32_t pipelineIndex = 0;

            switch (g_depthStencil->sampleCount)
            {
            case RenderSampleCount::COUNT_2:
                pipelineIndex = 0;
                break;
            case RenderSampleCount::COUNT_4:
                pipelineIndex = 1;
                break;
            case RenderSampleCount::COUNT_8:
                pipelineIndex = 2;
                break;
            default:
                assert(false && "Unsupported MSAA sample count");
                break;
            }

            if (args.texture->framebuffer == nullptr)
            {
                RenderFramebufferDesc desc;
                desc.depthAttachment = args.texture->texture;
                args.texture->framebuffer = g_device->createFramebuffer(desc);
            }

            if (g_framebuffer != args.texture->framebuffer.get())
            {
                commandList->setFramebuffer(args.texture->framebuffer.get());
                g_framebuffer = args.texture->framebuffer.get();
            }

            bool oldHalfPixel = SetHalfPixel(false);
            FlushViewport();

            commandList->setPipeline(g_resolveMsaaDepthPipelines[pipelineIndex].get());
            commandList->setGraphicsPushConstants(0, &g_depthStencil->descriptorIndex, 0, sizeof(uint32_t));
            commandList->drawInstanced(6, 1, 0, 0);

            g_dirtyStates.renderTargetAndDepthStencil = true;
            g_dirtyStates.pipelineState = true;

            if (g_vulkan)
                g_dirtyStates.vertexShaderConstants = true;

            SetHalfPixel(oldHalfPixel);
        }
        else
        {
            commandList->resolveTexture(args.texture->texture, surface->texture);
        }
    }
    else
    {
        commandList->copyTexture(args.texture->texture, surface->texture);
    }

    AddBarrier(args.texture, RenderTextureLayout::SHADER_READ);
}

static void SetDefaultViewport(GuestDevice* device, GuestSurface* surface)
{
    if (surface != nullptr)
    {
        RenderCommand cmd;
        cmd.type = RenderCommandType::SetViewport;
        cmd.setViewport.x = 0.0f;
        cmd.setViewport.y = 0.0f;
        cmd.setViewport.width = float(surface->width);
        cmd.setViewport.height = float(surface->height);
        cmd.setViewport.minDepth = 0.0f;
        cmd.setViewport.maxDepth = 1.0f;
        g_renderQueue.enqueue(cmd);

        device->viewport.x = 0.0f;
        device->viewport.y = 0.0f;
        device->viewport.width = float(surface->width);
        device->viewport.height = float(surface->height);
        device->viewport.minZ = 0.0f;
        device->viewport.maxZ = 1.0f;
    }
}

static void SetRenderTarget(GuestDevice* device, uint32_t index, GuestSurface* renderTarget) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetRenderTarget;
    cmd.setRenderTarget.renderTarget = renderTarget;
    g_renderQueue.enqueue(cmd);

    SetDefaultViewport(device, renderTarget);
}

static void ProcSetRenderTarget(const RenderCommand& cmd)
{
    const auto& args = cmd.setRenderTarget;

    SetDirtyValue(g_dirtyStates.renderTargetAndDepthStencil, g_renderTarget, args.renderTarget);
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.renderTargetFormat, args.renderTarget != nullptr ? args.renderTarget->format : RenderFormat::UNKNOWN);
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.sampleCount, args.renderTarget != nullptr ? args.renderTarget->sampleCount : RenderSampleCount::COUNT_1);

    // When alpha to coverage is enabled, update the alpha test mode as it's dependent on sample count.
    SetAlphaTestMode((g_pipelineState.specConstants & (SPEC_CONSTANT_ALPHA_TEST | SPEC_CONSTANT_ALPHA_TO_COVERAGE)) != 0);
}

static void SetDepthStencilSurface(GuestDevice* device, GuestSurface* depthStencil) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetDepthStencilSurface;
    cmd.setDepthStencilSurface.depthStencil = depthStencil;
    g_renderQueue.enqueue(cmd);

    SetDefaultViewport(device, depthStencil);
}

static void ProcSetDepthStencilSurface(const RenderCommand& cmd)
{
    const auto& args = cmd.setDepthStencilSurface;

    SetDirtyValue(g_dirtyStates.renderTargetAndDepthStencil, g_depthStencil, args.depthStencil);
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.depthStencilFormat, args.depthStencil != nullptr ? args.depthStencil->format : RenderFormat::UNKNOWN);
}

static void SetFramebuffer(GuestSurface* renderTarget, GuestSurface* depthStencil, bool settingForClear)
{
    if (settingForClear || g_dirtyStates.renderTargetAndDepthStencil)
    {
        GuestSurface* framebufferContainer = nullptr;
        RenderTexture* framebufferKey = nullptr;

        if (renderTarget != nullptr && depthStencil != nullptr)
        {
            framebufferContainer = depthStencil; // Backbuffer texture changes per frame so we can't use the depth stencil as the key.
            framebufferKey = renderTarget->texture;
        }
        else if (renderTarget != nullptr && depthStencil == nullptr)
        {
            framebufferContainer = renderTarget;
            framebufferKey = renderTarget->texture; // Backbuffer texture changes per frame so we can't assume nullptr for it.
        }
        else if (renderTarget == nullptr && depthStencil != nullptr)
        {
            framebufferContainer = depthStencil;
            framebufferKey = nullptr;
        }

        auto& commandList = g_commandLists[g_frame];

        if (framebufferContainer != nullptr)
        {
            auto& framebuffer = framebufferContainer->framebuffers[framebufferKey];

            if (framebuffer == nullptr)
            {
                RenderFramebufferDesc desc;

                if (renderTarget != nullptr)
                {
                    desc.colorAttachments = const_cast<const RenderTexture**>(&renderTarget->texture);
                    desc.colorAttachmentsCount = 1;
                }

                if (depthStencil != nullptr)
                    desc.depthAttachment = depthStencil->texture;

                framebuffer = g_device->createFramebuffer(desc);
            }

            if (g_framebuffer != framebuffer.get())
            {
                commandList->setFramebuffer(framebuffer.get());
                g_framebuffer = framebuffer.get();
            }
        }
        else if (g_framebuffer != nullptr)
        {
            commandList->setFramebuffer(nullptr);
            g_framebuffer = nullptr;
        }

        g_dirtyStates.renderTargetAndDepthStencil = settingForClear;
    }
}

static void Clear(GuestDevice* device, uint32_t flags, uint32_t, be<float>* color, double z) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::Clear;
    cmd.clear.flags = flags;
    cmd.clear.color[0] = color[0];
    cmd.clear.color[1] = color[1];
    cmd.clear.color[2] = color[2];
    cmd.clear.color[3] = color[3];
    cmd.clear.z = float(z);
    g_renderQueue.enqueue(cmd);
}

static void ProcClear(const RenderCommand& cmd)
{
    const auto& args = cmd.clear;

    AddBarrier(g_renderTarget, RenderTextureLayout::COLOR_WRITE);
    AddBarrier(g_depthStencil, RenderTextureLayout::DEPTH_WRITE);
    FlushBarriers();

    bool canClearInOnePass = (g_renderTarget == nullptr) || (g_depthStencil == nullptr) ||
        (g_renderTarget->width == g_depthStencil->width && g_renderTarget->height == g_depthStencil->height);

    if (canClearInOnePass)
        SetFramebuffer(g_renderTarget, g_depthStencil, true);

    auto& commandList = g_commandLists[g_frame];

    if (g_renderTarget != nullptr && (args.flags & D3DCLEAR_TARGET) != 0)
    {
        if (!canClearInOnePass)
            SetFramebuffer(g_renderTarget, nullptr, true);

        commandList->clearColor(0, RenderColor(args.color[0], args.color[1], args.color[2], args.color[3]));
    }

    if (g_depthStencil != nullptr && (args.flags & D3DCLEAR_ZBUFFER) != 0)
    {
        if (!canClearInOnePass)
            SetFramebuffer(nullptr, g_depthStencil, true);

        commandList->clearDepth(true, args.z);
    }
}

static void SetViewport(GuestDevice* device, GuestViewport* viewport)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetViewport;
    cmd.setViewport.x = viewport->x;
    cmd.setViewport.y = viewport->y;
    cmd.setViewport.width = viewport->width;
    cmd.setViewport.height = viewport->height;
    cmd.setViewport.minDepth = viewport->minZ;
    cmd.setViewport.maxDepth = viewport->maxZ;
    g_renderQueue.enqueue(cmd);

    device->viewport.x = float(viewport->x);
    device->viewport.y = float(viewport->y);
    device->viewport.width = float(viewport->width);
    device->viewport.height = float(viewport->height);
    device->viewport.minZ = viewport->minZ;
    device->viewport.maxZ = viewport->maxZ;
}

static void ProcSetViewport(const RenderCommand& cmd)
{
    const auto& args = cmd.setViewport;

    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.x, args.x);
    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.y, args.y);
    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.width, args.width);
    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.height, args.height);
    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.minDepth, args.minDepth);
    SetDirtyValue<float>(g_dirtyStates.viewport, g_viewport.maxDepth, args.maxDepth);
    
    uint32_t specConstants = g_pipelineState.specConstants;
    if (args.minDepth > args.maxDepth)
        specConstants |= SPEC_CONSTANT_REVERSE_Z;
    else 
        specConstants &= ~SPEC_CONSTANT_REVERSE_Z;

    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.specConstants, specConstants);

    g_dirtyStates.scissorRect |= g_dirtyStates.viewport;
}

static void SetTexture(GuestDevice* device, uint32_t index, GuestTexture* texture) 
{
    if (Config::ControllerIcons == EControllerIcons::PlayStation && texture != nullptr && texture->patchedTexture != nullptr)
        texture = texture->patchedTexture.get();

    RenderCommand cmd;
    cmd.type = RenderCommandType::SetTexture;
    cmd.setTexture.index = index;
    cmd.setTexture.texture = texture;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetTexture(const RenderCommand& cmd)
{
    const auto& args = cmd.setTexture;

    AddBarrier(args.texture, RenderTextureLayout::SHADER_READ);

    auto viewDimension = args.texture != nullptr ? args.texture->viewDimension : RenderTextureViewDimension::UNKNOWN;

    SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.texture2DIndices[args.index],
        viewDimension == RenderTextureViewDimension::TEXTURE_2D ? args.texture->descriptorIndex : TEXTURE_DESCRIPTOR_NULL_TEXTURE_2D);

    SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.texture3DIndices[args.index], args.texture != nullptr &&
        viewDimension == RenderTextureViewDimension::TEXTURE_3D ? args.texture->descriptorIndex : TEXTURE_DESCRIPTOR_NULL_TEXTURE_3D); 
    
    SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.textureCubeIndices[args.index], args.texture != nullptr &&
        viewDimension == RenderTextureViewDimension::TEXTURE_CUBE ? args.texture->descriptorIndex : TEXTURE_DESCRIPTOR_NULL_TEXTURE_CUBE);
}

static void SetScissorRect(GuestDevice* device, GuestRect* rect)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetScissorRect;
    cmd.setScissorRect.top = rect->top;
    cmd.setScissorRect.left = rect->left;
    cmd.setScissorRect.bottom = rect->bottom;
    cmd.setScissorRect.right = rect->right;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetScissorRect(const RenderCommand& cmd)
{
    const auto& args = cmd.setScissorRect;

    SetDirtyValue<int32_t>(g_dirtyStates.scissorRect, g_scissorRect.top, args.top);
    SetDirtyValue<int32_t>(g_dirtyStates.scissorRect, g_scissorRect.left, args.left);
    SetDirtyValue<int32_t>(g_dirtyStates.scissorRect, g_scissorRect.bottom, args.bottom);
    SetDirtyValue<int32_t>(g_dirtyStates.scissorRect, g_scissorRect.right, args.right);
}

static Mutex g_compiledSpecConstantLibraryBlobMutex;
static ankerl::unordered_dense::map<uint32_t, ComPtr<IDxcBlob>> g_compiledSpecConstantLibraryBlobs;

static RenderShader* GetOrLinkShader(GuestShader* guestShader, uint32_t specConstants)
{
    if (g_vulkan ||
        guestShader->shaderCacheEntry == nullptr || 
        guestShader->shaderCacheEntry->specConstantsMask == 0)
    {
        std::lock_guard lock(guestShader->mutex);

        if (guestShader->shader == nullptr)
        {
            assert(guestShader->shaderCacheEntry != nullptr);

            if (g_vulkan)
            {
                auto compressedSpirvData = g_shaderCache.get() + guestShader->shaderCacheEntry->spirvOffset;

                std::vector<uint8_t> decoded(smolv::GetDecodedBufferSize(compressedSpirvData, guestShader->shaderCacheEntry->spirvSize));
                bool result = smolv::Decode(compressedSpirvData, guestShader->shaderCacheEntry->spirvSize, decoded.data(), decoded.size());
                assert(result);

                guestShader->shader = g_device->createShader(decoded.data(), decoded.size(), "main", RenderShaderFormat::SPIRV);
            }
            else
            {
                guestShader->shader = g_device->createShader(g_shaderCache.get() + guestShader->shaderCacheEntry->dxilOffset, 
                    guestShader->shaderCacheEntry->dxilSize, "main", RenderShaderFormat::DXIL);
            }
        }

        return guestShader->shader.get();
    }

    specConstants &= guestShader->shaderCacheEntry->specConstantsMask;

    RenderShader* shader;
    {
        std::lock_guard lock(guestShader->mutex);
        shader = guestShader->linkedShaders[specConstants].get();
    }

    if (shader == nullptr)
    {
        thread_local ComPtr<IDxcCompiler3> s_dxcCompiler;
        thread_local ComPtr<IDxcLinker> s_dxcLinker;
        thread_local ComPtr<IDxcUtils> s_dxcUtils;

        wchar_t specConstantsLibName[0x100];
        swprintf_s(specConstantsLibName, L"SpecConstants_%d", specConstants);

        ComPtr<IDxcBlob> specConstantLibraryBlob;
        {
            std::lock_guard lock(g_compiledSpecConstantLibraryBlobMutex);
            specConstantLibraryBlob = g_compiledSpecConstantLibraryBlobs[specConstants];
        }

        if (specConstantLibraryBlob == nullptr)
        {
            if (s_dxcCompiler == nullptr)
            {
                HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(s_dxcCompiler.GetAddressOf()));
                assert(SUCCEEDED(hr) && s_dxcCompiler != nullptr);
            }

            char libraryHlsl[0x100];
            sprintf_s(libraryHlsl, "export uint g_SpecConstants() { return %d; }", specConstants);

            DxcBuffer buffer{};
            buffer.Ptr = libraryHlsl;
            buffer.Size = strlen(libraryHlsl);

            const wchar_t* args[1];
            args[0] = L"-T lib_6_3";

            ComPtr<IDxcResult> result;
            HRESULT hr = s_dxcCompiler->Compile(&buffer, args, std::size(args), nullptr, IID_PPV_ARGS(result.GetAddressOf()));
            assert(SUCCEEDED(hr) && result != nullptr);

            hr = result->GetResult(specConstantLibraryBlob.GetAddressOf());
            assert(SUCCEEDED(hr) && specConstantLibraryBlob != nullptr);

            std::lock_guard lock(g_compiledSpecConstantLibraryBlobMutex);
            g_compiledSpecConstantLibraryBlobs.emplace(specConstants, specConstantLibraryBlob);
        }

        if (s_dxcLinker == nullptr)
        {
            HRESULT hr = DxcCreateInstance(CLSID_DxcLinker, IID_PPV_ARGS(s_dxcLinker.GetAddressOf()));
            assert(SUCCEEDED(hr) && s_dxcLinker != nullptr);
        }

        s_dxcLinker->RegisterLibrary(specConstantsLibName, specConstantLibraryBlob.Get());

        wchar_t shaderLibName[0x100];
        swprintf_s(shaderLibName, L"Shader_%d", guestShader->shaderCacheEntry->dxilOffset);

        ComPtr<IDxcBlobEncoding> shaderLibraryBlob;
        {
            std::lock_guard lock(guestShader->mutex);
            shaderLibraryBlob = guestShader->libraryBlob;
        }

        if (shaderLibraryBlob == nullptr)
        {
            if (s_dxcUtils == nullptr)
            {
                HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(s_dxcUtils.GetAddressOf()));
                assert(SUCCEEDED(hr) && s_dxcUtils != nullptr);
            }

            HRESULT hr = s_dxcUtils->CreateBlobFromPinned(
                g_shaderCache.get() + guestShader->shaderCacheEntry->dxilOffset,
                guestShader->shaderCacheEntry->dxilSize,
                DXC_CP_ACP,
                shaderLibraryBlob.GetAddressOf());

            assert(SUCCEEDED(hr) && shaderLibraryBlob != nullptr);

            std::lock_guard lock(guestShader->mutex);
            guestShader->libraryBlob = shaderLibraryBlob;
        }

        s_dxcLinker->RegisterLibrary(shaderLibName, shaderLibraryBlob.Get());

        const wchar_t* libraryNames[] = { specConstantsLibName, shaderLibName };

        ComPtr<IDxcOperationResult> result;
        HRESULT hr = s_dxcLinker->Link(L"main", guestShader->type == ResourceType::VertexShader ? L"vs_6_0" : L"ps_6_0",
            libraryNames, std::size(libraryNames), nullptr, 0, result.GetAddressOf());

        assert(SUCCEEDED(hr) && result != nullptr);

        ComPtr<IDxcBlob> blob;
        hr = result->GetResult(blob.GetAddressOf());
        assert(SUCCEEDED(hr) && blob != nullptr);

        {
            std::lock_guard lock(guestShader->mutex);

            auto& linkedShader = guestShader->linkedShaders[specConstants];
            if (linkedShader == nullptr)
            {
                linkedShader = g_device->createShader(blob->GetBufferPointer(), blob->GetBufferSize(), "main", RenderShaderFormat::DXIL);
                guestShader->shaderBlobs.push_back(std::move(blob));
            }

            shader = linkedShader.get();
        }        
    }

    return shader;
}

static void SanitizePipelineState(PipelineState& pipelineState)
{
    if (!pipelineState.zEnable)
    {
        pipelineState.zWriteEnable = false;
        pipelineState.zFunc = RenderComparisonFunction::LESS;
        pipelineState.slopeScaledDepthBias = 0.0f;
        pipelineState.depthBias = 0;
        pipelineState.depthStencilFormat = RenderFormat::UNKNOWN;
    }

    if (pipelineState.slopeScaledDepthBias == 0.0f)
        pipelineState.slopeScaledDepthBias = 0.0f; // Remove sign.

    if (!pipelineState.colorWriteEnable)
    {
        pipelineState.alphaBlendEnable = false;
        pipelineState.renderTargetFormat = RenderFormat::UNKNOWN;
    }

    if (!pipelineState.alphaBlendEnable)
    {
        pipelineState.srcBlend = RenderBlend::ONE;
        pipelineState.destBlend = RenderBlend::ZERO;
        pipelineState.blendOp = RenderBlendOperation::ADD;
        pipelineState.srcBlendAlpha = RenderBlend::ONE;
        pipelineState.destBlendAlpha = RenderBlend::ZERO;
        pipelineState.blendOpAlpha = RenderBlendOperation::ADD;
    }

    uint32_t specConstantsMask = 0;
    if (pipelineState.vertexShader->shaderCacheEntry != nullptr)
        specConstantsMask |= pipelineState.vertexShader->shaderCacheEntry->specConstantsMask;

    if (pipelineState.pixelShader != nullptr && pipelineState.pixelShader->shaderCacheEntry != nullptr)
        specConstantsMask |= pipelineState.pixelShader->shaderCacheEntry->specConstantsMask;

    pipelineState.specConstants &= specConstantsMask;
}

static std::unique_ptr<RenderPipeline> CreateGraphicsPipeline(const PipelineState& pipelineState)
{
#ifdef ASYNC_PSO_DEBUG
    ++g_pipelinesCurrentlyCompiling;
#endif

    RenderGraphicsPipelineDesc desc;
    desc.pipelineLayout = g_pipelineLayout.get();
    desc.vertexShader = GetOrLinkShader(pipelineState.vertexShader, pipelineState.specConstants);
    desc.pixelShader = pipelineState.pixelShader != nullptr ? GetOrLinkShader(pipelineState.pixelShader, pipelineState.specConstants) : nullptr;
    desc.depthFunction = pipelineState.zFunc;
    desc.depthEnabled = pipelineState.zEnable;
    desc.depthWriteEnabled = pipelineState.zWriteEnable;
    desc.depthBias = pipelineState.depthBias;
    desc.slopeScaledDepthBias = pipelineState.slopeScaledDepthBias;
    desc.depthClipEnabled = true;
    desc.primitiveTopology = pipelineState.primitiveTopology;
    desc.cullMode = pipelineState.cullMode;
    desc.renderTargetFormat[0] = pipelineState.renderTargetFormat;
    desc.renderTargetBlend[0].blendEnabled = pipelineState.alphaBlendEnable;
    desc.renderTargetBlend[0].srcBlend = pipelineState.srcBlend;
    desc.renderTargetBlend[0].dstBlend = pipelineState.destBlend;
    desc.renderTargetBlend[0].blendOp = pipelineState.blendOp;
    desc.renderTargetBlend[0].srcBlendAlpha = pipelineState.srcBlendAlpha;
    desc.renderTargetBlend[0].dstBlendAlpha = pipelineState.destBlendAlpha;
    desc.renderTargetBlend[0].blendOpAlpha = pipelineState.blendOpAlpha;
    desc.renderTargetBlend[0].renderTargetWriteMask = pipelineState.colorWriteEnable;
    desc.renderTargetCount = pipelineState.renderTargetFormat != RenderFormat::UNKNOWN ? 1 : 0;
    desc.depthTargetFormat = pipelineState.depthStencilFormat;
    desc.multisampling.sampleCount = pipelineState.sampleCount;
    desc.alphaToCoverageEnabled = pipelineState.enableAlphaToCoverage;
    desc.inputElements = pipelineState.vertexDeclaration->inputElements.get();
    desc.inputElementsCount = pipelineState.vertexDeclaration->inputElementCount;
    
    RenderSpecConstant specConstant{};
    specConstant.value = pipelineState.specConstants;
    
    if (pipelineState.specConstants != 0)
    {
        desc.specConstants = &specConstant;
        desc.specConstantsCount = 1;
    }
    
    RenderInputSlot inputSlots[16]{};
    uint32_t inputSlotIndices[16]{};
    uint32_t inputSlotCount = 0;
    
    for (size_t i = 0; i < pipelineState.vertexDeclaration->inputElementCount; i++)
    {
        auto& inputElement = pipelineState.vertexDeclaration->inputElements[i];
        auto& inputSlotIndex = inputSlotIndices[inputElement.slotIndex];
    
        if (inputSlotIndex == NULL)
            inputSlotIndex = ++inputSlotCount;
    
        auto& inputSlot = inputSlots[inputSlotIndex - 1];
        inputSlot.index = inputElement.slotIndex;
        inputSlot.stride = pipelineState.vertexStrides[inputElement.slotIndex];
    
        if (pipelineState.instancing && inputElement.slotIndex != 0 && inputElement.slotIndex != 15)
            inputSlot.classification = RenderInputSlotClassification::PER_INSTANCE_DATA;
        else
            inputSlot.classification = RenderInputSlotClassification::PER_VERTEX_DATA;
    }
    
    desc.inputSlots = inputSlots;
    desc.inputSlotsCount = inputSlotCount;
    
    auto pipeline = g_device->createGraphicsPipeline(desc);

#ifdef ASYNC_PSO_DEBUG
    --g_pipelinesCurrentlyCompiling;
#endif

    return pipeline;
}

static RenderPipeline* CreateGraphicsPipelineInRenderThread(PipelineState pipelineState)
{
    SanitizePipelineState(pipelineState);

    XXH64_hash_t hash = XXH3_64bits(&pipelineState, sizeof(pipelineState));
    auto& pipeline = g_pipelines[hash];
    if (pipeline == nullptr)
    {
        pipeline = CreateGraphicsPipeline(pipelineState);

#ifdef ASYNC_PSO_DEBUG
        bool loading = *reinterpret_cast<bool*>(g_memory.Translate(0x83367A4C));

        if (loading)
            ++g_pipelinesCreatedAsynchronously;
        else
            ++g_pipelinesCreatedInRenderThread;

        pipeline->setName(std::format("{} {} {} {:X}", loading ? "ASYNC" : "",
            pipelineState.vertexShader->name, pipelineState.pixelShader != nullptr ? pipelineState.pixelShader->name : "<none>", hash));
        
        if (!loading)
        {
            std::lock_guard lock(g_debugMutex);
            g_pipelineDebugText = std::format(
                "PipelineState {:X}:\n"
                "  vertexShader: {}\n"
                "  pixelShader: {}\n"
                "  vertexDeclaration: {:X}\n"
                "  instancing: {}\n"
                "  zEnable: {}\n"
                "  zWriteEnable: {}\n"
                "  srcBlend: {}\n"
                "  destBlend: {}\n"
                "  cullMode: {}\n"
                "  zFunc: {}\n"
                "  alphaBlendEnable: {}\n"
                "  blendOp: {}\n"
                "  slopeScaledDepthBias: {}\n"
                "  depthBias: {}\n"
                "  srcBlendAlpha: {}\n"
                "  destBlendAlpha: {}\n"
                "  blendOpAlpha: {}\n"
                "  colorWriteEnable: {:X}\n"
                "  primitiveTopology: {}\n"
                "  vertexStrides[0]: {}\n"
                "  vertexStrides[1]: {}\n"
                "  vertexStrides[2]: {}\n"
                "  vertexStrides[3]: {}\n"
                "  renderTargetFormat: {}\n"
                "  depthStencilFormat: {}\n"
                "  sampleCount: {}\n"
                "  enableAlphaToCoverage: {}\n"
                "  specConstants: {:X}\n",
                hash,
                pipelineState.vertexShader->name,
                pipelineState.pixelShader != nullptr ? pipelineState.pixelShader->name : "<none>",
                reinterpret_cast<size_t>(pipelineState.vertexDeclaration),
                pipelineState.instancing,
                pipelineState.zEnable,
                pipelineState.zWriteEnable,
                magic_enum::enum_name(pipelineState.srcBlend),
                magic_enum::enum_name(pipelineState.destBlend),
                magic_enum::enum_name(pipelineState.cullMode),
                magic_enum::enum_name(pipelineState.zFunc),
                pipelineState.alphaBlendEnable,
                magic_enum::enum_name(pipelineState.blendOp),
                pipelineState.slopeScaledDepthBias,
                pipelineState.depthBias,
                magic_enum::enum_name(pipelineState.srcBlendAlpha),
                magic_enum::enum_name(pipelineState.destBlendAlpha),
                magic_enum::enum_name(pipelineState.blendOpAlpha),
                pipelineState.colorWriteEnable,
                magic_enum::enum_name(pipelineState.primitiveTopology),
                pipelineState.vertexStrides[0],
                pipelineState.vertexStrides[1],
                pipelineState.vertexStrides[2],
                pipelineState.vertexStrides[3],
                magic_enum::enum_name(pipelineState.renderTargetFormat),
                magic_enum::enum_name(pipelineState.depthStencilFormat),
                pipelineState.sampleCount,
                pipelineState.enableAlphaToCoverage,
                pipelineState.specConstants)
                + g_pipelineDebugText;
        }
#endif

#ifdef PSO_CACHING
        std::lock_guard lock(g_pipelineCacheMutex);
        g_pipelineStatesToCache.push_back(pipelineState);
#endif
    }
    
    return pipeline.get();
}

static RenderTextureAddressMode ConvertTextureAddressMode(size_t value)
{
    switch (value)
    {
    case D3DTADDRESS_WRAP:
        return RenderTextureAddressMode::WRAP;
    case D3DTADDRESS_MIRROR:
        return RenderTextureAddressMode::MIRROR;
    case D3DTADDRESS_CLAMP:
        return RenderTextureAddressMode::CLAMP;
    case D3DTADDRESS_MIRRORONCE:
        return RenderTextureAddressMode::MIRROR_ONCE;
    case D3DTADDRESS_BORDER:
        return RenderTextureAddressMode::BORDER;
    default:
        assert(false && "Unknown texture address mode");
        return RenderTextureAddressMode::UNKNOWN;
    }
}

static RenderFilter ConvertTextureFilter(uint32_t value)
{
    switch (value)
    {
    case D3DTEXF_POINT:
    case D3DTEXF_NONE:
        return RenderFilter::NEAREST;
    case D3DTEXF_LINEAR:
        return RenderFilter::LINEAR;
    default:
        assert(false && "Unknown texture filter");
        return RenderFilter::UNKNOWN;
    }
}

static RenderBorderColor ConvertBorderColor(uint32_t value)
{
    switch (value)
    {
    case 0:
        return RenderBorderColor::TRANSPARENT_BLACK;
    case 1:
        return RenderBorderColor::OPAQUE_WHITE;
    default:
        assert(false && "Unknown border color");
        return RenderBorderColor::UNKNOWN;
    }
}

struct LocalRenderCommandQueue
{
    RenderCommand commands[20];
    uint32_t count = 0;

    RenderCommand& enqueue()
    {
        assert(count < std::size(commands));
        return commands[count++];
    }

    void submit()
    {
        g_renderQueue.enqueue_bulk(commands, count);
    }
};

static void FlushRenderStateForMainThread(GuestDevice* device, LocalRenderCommandQueue& queue)
{
    constexpr size_t BOOL_MASK = 0x100000000000000ull;
    if ((device->dirtyFlags[4].get() & BOOL_MASK) != 0)
    {
        auto& cmd = queue.enqueue();
        cmd.type = RenderCommandType::SetBooleans;
        cmd.setBooleans.booleans = (device->vertexShaderBoolConstants[0].get() & 0xFF) | ((device->pixelShaderBoolConstants[0].get() & 0xFF) << 16);

        device->dirtyFlags[4] = device->dirtyFlags[4].get() & ~BOOL_MASK;
    }

    for (uint32_t i = 0; i < 16; i++)
    {
        const size_t mask = 0x8000000000000000ull >> (i + 32);
        if (device->dirtyFlags[3].get() & mask)
        {
            auto& cmd = queue.enqueue();
            cmd.type = RenderCommandType::SetSamplerState;
            cmd.setSamplerState.index = i;
            cmd.setSamplerState.data0 = device->samplerStates[i].data[0];
            cmd.setSamplerState.data3 = device->samplerStates[i].data[3];
            cmd.setSamplerState.data5 = device->samplerStates[i].data[5];

            device->dirtyFlags[3] = device->dirtyFlags[3].get() & ~mask;
        }
    }

    if (g_dirtyStates.vertexShaderConstants || device->dirtyFlags[0] != 0)
    {
        WaitForRenderThread();

        auto& cmd = queue.enqueue();
        cmd.type = RenderCommandType::SetVertexShaderConstants;
        cmd.setVertexShaderConstants.allocation = g_uploadAllocators[g_frame].allocate<true>(device->vertexShaderFloatConstants, 0x1000, 0x100);

        device->dirtyFlags[0] = 0;
    }

    if (g_dirtyStates.pixelShaderConstants || device->dirtyFlags[1] != 0)
    {
        WaitForRenderThread();

        auto& cmd = queue.enqueue();
        cmd.type = RenderCommandType::SetPixelShaderConstants;
        cmd.setPixelShaderConstants.allocation = g_uploadAllocators[g_frame].allocate<true>(device->pixelShaderFloatConstants, 0xE00, 0x100);

        device->dirtyFlags[1] = 0;
    }
}

static void ProcSetBooleans(const RenderCommand& cmd)
{
    SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.booleans, cmd.setBooleans.booleans);
}

static void ProcSetSamplerState(const RenderCommand& cmd)
{
    const auto& args = cmd.setSamplerState;

    const auto addressU = ConvertTextureAddressMode((args.data0 >> 10) & 0x7);
    const auto addressV = ConvertTextureAddressMode((args.data0 >> 13) & 0x7);
    const auto addressW = ConvertTextureAddressMode((args.data0 >> 16) & 0x7);
    auto magFilter = ConvertTextureFilter((args.data3 >> 19) & 0x3);
    auto minFilter = ConvertTextureFilter((args.data3 >> 21) & 0x3);
    auto mipFilter = ConvertTextureFilter((args.data3 >> 23) & 0x3);
    const auto borderColor = ConvertBorderColor(args.data5 & 0x3);

    bool anisotropyEnabled = mipFilter == RenderFilter::LINEAR;
    if (anisotropyEnabled)
    {
        magFilter = RenderFilter::LINEAR;
        minFilter = RenderFilter::LINEAR;
    }

    auto& samplerDesc = g_samplerDescs[args.index];

    bool dirty = false;

    SetDirtyValue(dirty, samplerDesc.addressU, addressU);
    SetDirtyValue(dirty, samplerDesc.addressV, addressV);
    SetDirtyValue(dirty, samplerDesc.addressW, addressW);
    SetDirtyValue(dirty, samplerDesc.minFilter, minFilter);
    SetDirtyValue(dirty, samplerDesc.magFilter, magFilter);
    SetDirtyValue(dirty, samplerDesc.mipmapMode, RenderMipmapMode(mipFilter));
    SetDirtyValue(dirty, samplerDesc.anisotropyEnabled, anisotropyEnabled);
    SetDirtyValue(dirty, samplerDesc.borderColor, borderColor);

    if (dirty)
    {
        auto& [descriptorIndex, sampler] = g_samplerStates[XXH3_64bits(&samplerDesc, sizeof(RenderSamplerDesc))];
        if (descriptorIndex == NULL)
        {
            descriptorIndex = g_samplerStates.size();
            sampler = g_device->createSampler(samplerDesc);

            g_samplerDescriptorSet->setSampler(descriptorIndex - 1, sampler.get());
        }

        SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.samplerIndices[args.index], descriptorIndex - 1);
    }
}

static void ProcSetVertexShaderConstants(const RenderCommand& cmd)
{
    SetRootDescriptor(cmd.setVertexShaderConstants.allocation, 0);
}

static void ProcSetPixelShaderConstants(const RenderCommand& cmd)
{
    SetRootDescriptor(cmd.setPixelShaderConstants.allocation, 1);
}

static void ProcAddPipeline(const RenderCommand& cmd)
{
    auto& args = cmd.addPipeline;
    auto& pipeline = g_pipelines[args.hash];

    if (pipeline == nullptr)
    {
        pipeline = std::unique_ptr<RenderPipeline>(args.pipeline);
#ifdef ASYNC_PSO_DEBUG
        ++g_pipelinesCreatedAsynchronously;
#endif
    }
    else
    {
#ifdef ASYNC_PSO_DEBUG
        ++g_pipelinesDropped;
#endif
        delete args.pipeline;
    }
}

static void FlushRenderStateForRenderThread()
{
    auto renderTarget = g_pipelineState.colorWriteEnable ? g_renderTarget : nullptr;
    auto depthStencil = g_pipelineState.zEnable ? g_depthStencil : nullptr;

    AddBarrier(renderTarget, RenderTextureLayout::COLOR_WRITE);
    AddBarrier(depthStencil, RenderTextureLayout::DEPTH_WRITE);
    FlushBarriers();
    SetFramebuffer(renderTarget, depthStencil, false);
    FlushViewport();

    auto& commandList = g_commandLists[g_frame];

    if (g_dirtyStates.pipelineState)
        commandList->setPipeline(CreateGraphicsPipelineInRenderThread(g_pipelineState));

    if (g_dirtyStates.sharedConstants)
    {
        auto sharedConstants = g_uploadAllocators[g_frame].allocate<false>(&g_sharedConstants, sizeof(g_sharedConstants), 0x100);
        SetRootDescriptor(sharedConstants, 2);
    }

    if (g_dirtyStates.vertexStreamFirst <= g_dirtyStates.vertexStreamLast)
    {
        commandList->setVertexBuffers(
            g_dirtyStates.vertexStreamFirst,
            g_vertexBufferViews + g_dirtyStates.vertexStreamFirst,
            g_dirtyStates.vertexStreamLast - g_dirtyStates.vertexStreamFirst + 1,
            g_inputSlots + g_dirtyStates.vertexStreamFirst);
    }

    if (g_dirtyStates.indices && (!g_vulkan || g_indexBufferView.buffer.ref != nullptr))
        commandList->setIndexBuffer(&g_indexBufferView);

    g_dirtyStates = DirtyStates(false);
}

static RenderPrimitiveTopology ConvertPrimitiveType(uint32_t primitiveType)
{
    switch (primitiveType)
    {
    case D3DPT_POINTLIST:
        return RenderPrimitiveTopology::POINT_LIST;
    case D3DPT_LINELIST:
        return RenderPrimitiveTopology::LINE_LIST;
    case D3DPT_LINESTRIP:
        return RenderPrimitiveTopology::LINE_STRIP;
    case D3DPT_TRIANGLELIST:
    case D3DPT_QUADLIST:
        return RenderPrimitiveTopology::TRIANGLE_LIST;
    case D3DPT_TRIANGLESTRIP:
        return RenderPrimitiveTopology::TRIANGLE_STRIP;
    case D3DPT_TRIANGLEFAN:
        return g_triangleFanSupported ? RenderPrimitiveTopology::TRIANGLE_FAN : RenderPrimitiveTopology::TRIANGLE_LIST;
    default:
        assert(false && "Unknown primitive type");
        return RenderPrimitiveTopology::UNKNOWN;
    }
}

static void SetPrimitiveType(uint32_t primitiveType)
{
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.primitiveTopology, ConvertPrimitiveType(primitiveType));
}

static uint32_t CheckInstancing()
{
    uint32_t indexCount = 0;

    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.instancing, g_pipelineState.vertexDeclaration->indexVertexStream != 0);
    if (g_pipelineState.instancing)
    {
        // Index buffer is passed as a vertex stream
        indexCount = g_vertexBufferViews[g_pipelineState.vertexDeclaration->indexVertexStream].size / 4;
    }

    return indexCount;
}

static void DrawPrimitive(GuestDevice* device, uint32_t primitiveType, uint32_t startVertex, uint32_t primitiveCount) 
{
    LocalRenderCommandQueue queue;
    FlushRenderStateForMainThread(device, queue);

    auto& cmd = queue.enqueue();
    cmd.type = RenderCommandType::DrawPrimitive;
    cmd.drawPrimitive.primitiveType = primitiveType;
    cmd.drawPrimitive.startVertex = startVertex;
    cmd.drawPrimitive.primitiveCount = primitiveCount;

    queue.submit();
}

static void ProcDrawPrimitive(const RenderCommand& cmd)
{
    const auto& args = cmd.drawPrimitive;

    SetPrimitiveType(args.primitiveType);

    uint32_t indexCount = CheckInstancing();
    if (indexCount > 0)
    {
        auto& vertexBufferView = g_vertexBufferViews[g_pipelineState.vertexDeclaration->indexVertexStream];

        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.buffer, vertexBufferView.buffer);
        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.size, vertexBufferView.size);
        SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.format, RenderFormat::R32_UINT);
    }

    FlushRenderStateForRenderThread();

    auto& commandList = g_commandLists[g_frame];

    if (indexCount > 0)
        commandList->drawIndexedInstanced(indexCount, args.primitiveCount / indexCount, 0, 0, 0);
    else
        commandList->drawInstanced(args.primitiveCount, 1, args.startVertex, 0);
}

static void DrawIndexedPrimitive(GuestDevice* device, uint32_t primitiveType, int32_t baseVertexIndex, uint32_t startIndex, uint32_t primCount)
{
    LocalRenderCommandQueue queue;
    FlushRenderStateForMainThread(device, queue);

    auto& cmd = queue.enqueue();
    cmd.type = RenderCommandType::DrawIndexedPrimitive;
    cmd.drawIndexedPrimitive.primitiveType = primitiveType;
    cmd.drawIndexedPrimitive.baseVertexIndex = baseVertexIndex;
    cmd.drawIndexedPrimitive.startIndex = startIndex;
    cmd.drawIndexedPrimitive.primCount = primCount;

    queue.submit();
}

static void ProcDrawIndexedPrimitive(const RenderCommand& cmd)
{
    const auto& args = cmd.drawIndexedPrimitive;

    CheckInstancing();
    SetPrimitiveType(args.primitiveType);
    FlushRenderStateForRenderThread();

    g_commandLists[g_frame]->drawIndexedInstanced(args.primCount, 1, args.startIndex, args.baseVertexIndex, 0);
}

static void DrawPrimitiveUP(GuestDevice* device, uint32_t primitiveType, uint32_t primitiveCount, void* vertexStreamZeroData, uint32_t vertexStreamZeroStride)
{
    LocalRenderCommandQueue queue;
    FlushRenderStateForMainThread(device, queue);
    WaitForRenderThread();

    auto& cmd = queue.enqueue();
    cmd.type = RenderCommandType::DrawPrimitiveUP;
    cmd.drawPrimitiveUP.primitiveType = primitiveType;
    cmd.drawPrimitiveUP.primitiveCount = primitiveCount;
    cmd.drawPrimitiveUP.vertexStreamZeroData = g_uploadAllocators[g_frame].allocate<true>(reinterpret_cast<uint32_t*>(vertexStreamZeroData), primitiveCount * vertexStreamZeroStride, 0x4);
    cmd.drawPrimitiveUP.vertexStreamZeroStride = vertexStreamZeroStride;
    
    queue.submit();
}

static void ProcDrawPrimitiveUP(const RenderCommand& cmd)
{
    const auto& args = cmd.drawPrimitiveUP;

    CheckInstancing();
    SetPrimitiveType(args.primitiveType);
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.vertexStrides[0], uint8_t(args.vertexStreamZeroStride));

    auto& vertexBufferView = g_vertexBufferViews[0];
    vertexBufferView.size = args.primitiveCount * args.vertexStreamZeroStride;
    vertexBufferView.buffer = args.vertexStreamZeroData.buffer->at(args.vertexStreamZeroData.offset);
    g_inputSlots[0].stride = args.vertexStreamZeroStride;
    g_dirtyStates.vertexStreamFirst = 0;

    uint32_t indexCount = 0;

    if (args.primitiveType == D3DPT_QUADLIST)
        indexCount = g_quadIndexData.prepare(args.primitiveCount);
    else if (!g_triangleFanSupported && args.primitiveType == D3DPT_TRIANGLEFAN)
        indexCount = g_triangleFanIndexData.prepare(args.primitiveCount);

    FlushRenderStateForRenderThread();

    if (indexCount != 0)
        g_commandLists[g_frame]->drawIndexedInstanced(indexCount, 1, 0, 0, 0);
    else
        g_commandLists[g_frame]->drawInstanced(args.primitiveCount, 1, 0, 0);
}

static const char* ConvertDeclUsage(uint32_t usage)
{
    switch (usage)
    {
    case D3DDECLUSAGE_POSITION:
        return "POSITION";
    case D3DDECLUSAGE_BLENDWEIGHT:
        return "BLENDWEIGHT";
    case D3DDECLUSAGE_BLENDINDICES:
        return "BLENDINDICES";
    case D3DDECLUSAGE_NORMAL:
        return "NORMAL";
    case D3DDECLUSAGE_PSIZE:
        return "PSIZE";
    case D3DDECLUSAGE_TEXCOORD:
        return "TEXCOORD";
    case D3DDECLUSAGE_TANGENT:
        return "TANGENT";
    case D3DDECLUSAGE_BINORMAL:
        return "BINORMAL";
    case D3DDECLUSAGE_TESSFACTOR:
        return "TESSFACTOR";
    case D3DDECLUSAGE_POSITIONT:
        return "POSITIONT";
    case D3DDECLUSAGE_COLOR:
        return "COLOR";
    case D3DDECLUSAGE_FOG:
        return "FOG";
    case D3DDECLUSAGE_DEPTH:
        return "DEPTH";
    case D3DDECLUSAGE_SAMPLE:
        return "SAMPLE";
    default:
        assert(false && "Unknown usage");
        return "UNKNOWN";
    }
}

static RenderFormat ConvertDeclType(uint32_t type)
{
    switch (type)
    {
    case D3DDECLTYPE_FLOAT1:
        return RenderFormat::R32_FLOAT;
    case D3DDECLTYPE_FLOAT2:
        return RenderFormat::R32G32_FLOAT;
    case D3DDECLTYPE_FLOAT3:
        return RenderFormat::R32G32B32_FLOAT;
    case D3DDECLTYPE_FLOAT4:
        return RenderFormat::R32G32B32A32_FLOAT;
    case D3DDECLTYPE_D3DCOLOR:
        return RenderFormat::B8G8R8A8_UNORM;
    case D3DDECLTYPE_UBYTE4:
    case D3DDECLTYPE_UBYTE4_2:
        return RenderFormat::R8G8B8A8_UINT;
    case D3DDECLTYPE_SHORT2:
        return RenderFormat::R16G16_SINT;
    case D3DDECLTYPE_SHORT4:
        return RenderFormat::R16G16B16A16_SINT;
    case D3DDECLTYPE_UBYTE4N:
    case D3DDECLTYPE_UBYTE4N_2:
        return RenderFormat::R8G8B8A8_UNORM;
    case D3DDECLTYPE_SHORT2N:
        return RenderFormat::R16G16_SNORM;
    case D3DDECLTYPE_SHORT4N:
        return RenderFormat::R16G16B16A16_SNORM;
    case D3DDECLTYPE_USHORT2N:
        return RenderFormat::R16G16_UNORM;
    case D3DDECLTYPE_USHORT4N:
        return RenderFormat::R16G16B16A16_UNORM;
    case D3DDECLTYPE_UINT1:
        return RenderFormat::R32_UINT;
    case D3DDECLTYPE_DEC3N_2:
    case D3DDECLTYPE_DEC3N_3:
        return RenderFormat::R32_UINT;
    case D3DDECLTYPE_FLOAT16_2:
        return RenderFormat::R16G16_FLOAT;
    case D3DDECLTYPE_FLOAT16_4:
        return RenderFormat::R16G16B16A16_FLOAT;
    default:
        assert(false && "Unknown type");
        return RenderFormat::UNKNOWN;
    }
}

static GuestVertexDeclaration* CreateVertexDeclarationWithoutAddRef(GuestVertexElement* vertexElements) 
{
    size_t vertexElementCount = 0;
    auto vertexElement = vertexElements;

    while (vertexElement->stream != 0xFF && vertexElement->type != D3DDECLTYPE_UNUSED)
    {
        vertexElement->padding = 0;
        ++vertexElement;
        ++vertexElementCount;
    }

    std::lock_guard lock(g_vertexDeclarationMutex);

    XXH64_hash_t hash = XXH3_64bits(vertexElements, vertexElementCount * sizeof(GuestVertexElement));
    auto& vertexDeclaration = g_vertexDeclarations[hash];

    if (vertexDeclaration == nullptr)
    {
        vertexDeclaration = g_userHeap.AllocPhysical<GuestVertexDeclaration>(ResourceType::VertexDeclaration);
        vertexDeclaration->hash = hash;

        static std::vector<RenderInputElement> inputElements;
        inputElements.clear();

        struct Location
        {
            uint32_t usage;
            uint32_t usageIndex;
            uint32_t location;
        };

        constexpr Location locations[] =
        {
            { D3DDECLUSAGE_POSITION, 0, 0 },
            { D3DDECLUSAGE_NORMAL, 0, 1 },
            { D3DDECLUSAGE_TANGENT, 0, 2 },
            { D3DDECLUSAGE_BINORMAL, 0, 3 },
            { D3DDECLUSAGE_TEXCOORD, 0, 4 },
            { D3DDECLUSAGE_TEXCOORD, 1, 5 },
            { D3DDECLUSAGE_TEXCOORD, 2, 6 },
            { D3DDECLUSAGE_TEXCOORD, 3, 7 },
            { D3DDECLUSAGE_COLOR, 0, 8 },
            { D3DDECLUSAGE_BLENDINDICES, 0, 9 },
            { D3DDECLUSAGE_BLENDWEIGHT, 0, 10 },
            { D3DDECLUSAGE_COLOR, 1, 11 },
            { D3DDECLUSAGE_TEXCOORD, 4, 12 },
            { D3DDECLUSAGE_TEXCOORD, 5, 13 },
            { D3DDECLUSAGE_TEXCOORD, 6, 14 },
            { D3DDECLUSAGE_TEXCOORD, 7, 15 },
            { D3DDECLUSAGE_POSITION, 1, 15 }
        };

        vertexElement = vertexElements;
        while (vertexElement->stream != 0xFF && vertexElement->type != D3DDECLTYPE_UNUSED)
        {
            if (vertexElement->usage == D3DDECLUSAGE_POSITION && vertexElement->usageIndex == 2)
            {
                ++vertexElement;
                continue;
            }

            auto& inputElement = inputElements.emplace_back();
            
            inputElement.semanticName = ConvertDeclUsage(vertexElement->usage);
            inputElement.semanticIndex = vertexElement->usageIndex;
            inputElement.location = ~0;

            for (auto& location : locations)
            {
                if (location.usage == vertexElement->usage && location.usageIndex == vertexElement->usageIndex)
                {
                    inputElement.location = location.location;
                    break;
                }
            }

            assert(inputElement.location != ~0);

            inputElement.format = ConvertDeclType(vertexElement->type);
            inputElement.slotIndex = vertexElement->stream;
            inputElement.alignedByteOffset = vertexElement->offset;

            switch (vertexElement->usage)
            {
            case D3DDECLUSAGE_POSITION:
                if (vertexElement->usageIndex == 1)
                    vertexDeclaration->indexVertexStream = vertexElement->stream;
                break;

            case D3DDECLUSAGE_NORMAL:
            case D3DDECLUSAGE_TANGENT:
            case D3DDECLUSAGE_BINORMAL:
                if (vertexElement->type == D3DDECLTYPE_FLOAT3)
                    inputElement.format = RenderFormat::R32G32B32_UINT;
                else
                    vertexDeclaration->hasR11G11B10Normal = true;
                break;

            case D3DDECLUSAGE_TEXCOORD:
                switch (vertexElement->type)
                {
                case D3DDECLTYPE_SHORT2:
                case D3DDECLTYPE_SHORT4:
                case D3DDECLTYPE_SHORT2N:
                case D3DDECLTYPE_SHORT4N:
                case D3DDECLTYPE_USHORT2N:
                case D3DDECLTYPE_USHORT4N:
                case D3DDECLTYPE_FLOAT16_2:
                case D3DDECLTYPE_FLOAT16_4:
                    vertexDeclaration->swappedTexcoords |= 1 << vertexElement->usageIndex;
                    break;
                }

                break;
            }

            ++vertexElement;
        }

        auto addInputElement = [&](uint32_t usage, uint32_t usageIndex)
            {
                uint32_t location = ~0;

                for (auto& alsoLocation : locations)
                {
                    if (alsoLocation.usage == usage && alsoLocation.usageIndex == usageIndex)
                    {
                        location = alsoLocation.location;
                        break;
                    }
                }

                assert(location != ~0);

                for (auto& inputElement : inputElements)
                {
                    if (inputElement.location == location)
                        return;
                }

                auto format = RenderFormat::R32_FLOAT;
                switch (usage)
                {
                case D3DDECLUSAGE_NORMAL:
                case D3DDECLUSAGE_TANGENT:
                case D3DDECLUSAGE_BINORMAL:
                case D3DDECLUSAGE_BLENDINDICES:
                    format = RenderFormat::R32_UINT;
                    break;
                }

                inputElements.emplace_back(ConvertDeclUsage(usage), usageIndex, location, format, 15, 0);
            };

        addInputElement(D3DDECLUSAGE_POSITION, 0);
        addInputElement(D3DDECLUSAGE_NORMAL, 0);
        addInputElement(D3DDECLUSAGE_TANGENT, 0);
        addInputElement(D3DDECLUSAGE_BINORMAL, 0);
        addInputElement(D3DDECLUSAGE_TEXCOORD, 0);
        addInputElement(D3DDECLUSAGE_TEXCOORD, 1);
        addInputElement(D3DDECLUSAGE_TEXCOORD, 2);
        addInputElement(D3DDECLUSAGE_TEXCOORD, 3);
        addInputElement(D3DDECLUSAGE_COLOR, 0);
        addInputElement(D3DDECLUSAGE_BLENDWEIGHT, 0);
        addInputElement(D3DDECLUSAGE_BLENDINDICES, 0);

        vertexDeclaration->inputElements = std::make_unique<RenderInputElement[]>(inputElements.size());
        std::copy(inputElements.begin(), inputElements.end(), vertexDeclaration->inputElements.get());

        vertexDeclaration->vertexElements = std::make_unique<GuestVertexElement[]>(vertexElementCount + 1);
        std::copy(vertexElements, vertexElements + vertexElementCount + 1, vertexDeclaration->vertexElements.get());

        vertexDeclaration->inputElementCount = uint32_t(inputElements.size());
        vertexDeclaration->vertexElementCount = vertexElementCount + 1;
    }

    vertexDeclaration->AddRef();
    return vertexDeclaration;
}

static GuestVertexDeclaration* CreateVertexDeclaration(GuestVertexElement* vertexElements)
{
    auto vertexDeclaration = CreateVertexDeclarationWithoutAddRef(vertexElements);
    vertexDeclaration->AddRef();
    return vertexDeclaration;
}

static void SetVertexDeclaration(GuestDevice* device, GuestVertexDeclaration* vertexDeclaration) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetVertexDeclaration;
    cmd.setVertexDeclaration.vertexDeclaration = vertexDeclaration;
    g_renderQueue.enqueue(cmd);

    device->vertexDeclaration = g_memory.MapVirtual(vertexDeclaration);
}

static void ProcSetVertexDeclaration(const RenderCommand& cmd)
{
    auto& args = cmd.setVertexDeclaration;

    if (args.vertexDeclaration != nullptr)
    {
        SetDirtyValue(g_dirtyStates.sharedConstants, g_sharedConstants.swappedTexcoords, args.vertexDeclaration->swappedTexcoords);

        uint32_t specConstants = g_pipelineState.specConstants;
        if (args.vertexDeclaration->hasR11G11B10Normal)
            specConstants |= SPEC_CONSTANT_R11G11B10_NORMAL;
        else
            specConstants &= ~SPEC_CONSTANT_R11G11B10_NORMAL;

        SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.specConstants, specConstants);
    }
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.vertexDeclaration, args.vertexDeclaration);
}

static ShaderCacheEntry* FindShaderCacheEntry(XXH64_hash_t hash)
{
    auto end = g_shaderCacheEntries + g_shaderCacheEntryCount;
    auto findResult = std::lower_bound(g_shaderCacheEntries, end, hash, [](ShaderCacheEntry& lhs, XXH64_hash_t rhs)
        {
            return lhs.hash < rhs;
        });

    return findResult != end && findResult->hash == hash ? findResult : nullptr;
}

static GuestShader* CreateShader(const be<uint32_t>* function, ResourceType resourceType)
{
    XXH64_hash_t hash = XXH3_64bits(function, function[1] + function[2]);

    auto findResult = FindShaderCacheEntry(hash);
    GuestShader* shader = nullptr;

    if (findResult != nullptr)
    {
        if (findResult->guestShader == nullptr)
        {
            shader = g_userHeap.AllocPhysical<GuestShader>(resourceType);
            shader->shaderCacheEntry = findResult;

            findResult->guestShader = shader;
        }
        else
        {
            shader = findResult->guestShader;
        }
    }

    if (shader == nullptr)
        shader = g_userHeap.AllocPhysical<GuestShader>(resourceType);
    else
        shader->AddRef();

    return shader;
}

static GuestShader* CreateVertexShader(const be<uint32_t>* function) 
{
    return CreateShader(function, ResourceType::VertexShader);
}

static void SetVertexShader(GuestDevice* device, GuestShader* shader)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetVertexShader;
    cmd.setVertexShader.shader = shader;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetVertexShader(const RenderCommand& cmd)
{
    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.vertexShader, cmd.setVertexShader.shader);
}

static void SetStreamSource(GuestDevice* device, uint32_t index, GuestBuffer* buffer, uint32_t offset, uint32_t stride) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetStreamSource;
    cmd.setStreamSource.index = index;
    cmd.setStreamSource.buffer = buffer;
    cmd.setStreamSource.offset = offset;
    cmd.setStreamSource.stride = stride;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetStreamSource(const RenderCommand& cmd)
{
    const auto& args = cmd.setStreamSource;

    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.vertexStrides[args.index], uint8_t(args.buffer != nullptr ? args.stride : 0));

    bool dirty = false;

    SetDirtyValue(dirty, g_vertexBufferViews[args.index].buffer, args.buffer != nullptr ? args.buffer->buffer->at(args.offset) : RenderBufferReference{});
    SetDirtyValue(dirty, g_vertexBufferViews[args.index].size, args.buffer != nullptr ? (args.buffer->dataSize - args.offset) : 0u);
    SetDirtyValue(dirty, g_inputSlots[args.index].stride, args.buffer != nullptr ? args.stride : 0u);

    if (dirty)
    {
        g_dirtyStates.vertexStreamFirst = std::min<uint8_t>(g_dirtyStates.vertexStreamFirst, args.index);
        g_dirtyStates.vertexStreamLast = std::max<uint8_t>(g_dirtyStates.vertexStreamLast, args.index);
    }
}

static void SetIndices(GuestDevice* device, GuestBuffer* buffer) 
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetIndices;
    cmd.setIndices.buffer = buffer;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetIndices(const RenderCommand& cmd)
{
    const auto& args = cmd.setIndices;

    SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.buffer, args.buffer != nullptr ? args.buffer->buffer->at(0) : RenderBufferReference{});
    SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.format, args.buffer != nullptr ? args.buffer->format : RenderFormat::R16_UINT);
    SetDirtyValue(g_dirtyStates.indices, g_indexBufferView.size, args.buffer != nullptr ? args.buffer->dataSize : 0u);
}

static GuestShader* CreatePixelShader(const be<uint32_t>* function)
{
    return CreateShader(function, ResourceType::PixelShader);
}

static void SetPixelShader(GuestDevice* device, GuestShader* shader)
{
    RenderCommand cmd;
    cmd.type = RenderCommandType::SetPixelShader;
    cmd.setPixelShader.shader = shader;
    g_renderQueue.enqueue(cmd);
}

static void ProcSetPixelShader(const RenderCommand& cmd)
{
    GuestShader* shader = cmd.setPixelShader.shader;
    if (shader != nullptr && 
        shader->shaderCacheEntry != nullptr &&
        shader->shaderCacheEntry->hash == 0x4294510C775F4EE8)
    {
        size_t shaderIndex = GAUSSIAN_BLUR_3X3;
        
        switch (Config::DepthOfFieldQuality)
        {
        case EDepthOfFieldQuality::Low:
            shaderIndex = GAUSSIAN_BLUR_3X3;
            break;    

        case EDepthOfFieldQuality::Medium:
            shaderIndex = GAUSSIAN_BLUR_5X5;
            break;  

        case EDepthOfFieldQuality::High:
            shaderIndex = GAUSSIAN_BLUR_7X7;
            break;   
        
        case EDepthOfFieldQuality::Ultra:
            shaderIndex = GAUSSIAN_BLUR_9X9;
            break;

        default:
        {
            size_t height = round(g_swapChain->getHeight() * Config::ResolutionScale);

            // Use the middle point between resolutions to have the transition less noticable.
            if (height >= ((2160 + 1440) / 2))
                shaderIndex = GAUSSIAN_BLUR_9X9;   
            else if (height >= ((1440 + 1080) / 2))
                shaderIndex = GAUSSIAN_BLUR_7X7;
            else if (height >= ((1080 + 720) / 2))
                shaderIndex = GAUSSIAN_BLUR_5X5;
            else
                shaderIndex = GAUSSIAN_BLUR_3X3;

            break;
        }
        }

        shader = g_gaussianBlurShaders[shaderIndex].get();
    }

    SetDirtyValue(g_dirtyStates.pipelineState, g_pipelineState.pixelShader, shader);
}

static std::thread g_renderThread([]
    {
        GuestThread::SetThreadName(GetCurrentThreadId(), "Render Thread");

        RenderCommand commands[32];

        while (true)
        {
            size_t count = g_renderQueue.wait_dequeue_bulk(commands, std::size(commands));
            for (size_t i = 0; i < count; i++)
            {
                auto& cmd = commands[i];
                switch (cmd.type)
                {
                case RenderCommandType::SetRenderState:           ProcSetRenderState(cmd); break;
                case RenderCommandType::DestructResource:         ProcDestructResource(cmd); break;
                case RenderCommandType::UnlockTextureRect:        ProcUnlockTextureRect(cmd); break;
                case RenderCommandType::UnlockBuffer16:           ProcUnlockBuffer16(cmd); break;
                case RenderCommandType::UnlockBuffer32:           ProcUnlockBuffer32(cmd); break;
                case RenderCommandType::DrawImGui:                ProcDrawImGui(cmd); break;
                case RenderCommandType::Present:                  ProcPresent(cmd); break;
                case RenderCommandType::StretchRect:              ProcStretchRect(cmd); break;
                case RenderCommandType::SetRenderTarget:          ProcSetRenderTarget(cmd); break;
                case RenderCommandType::SetDepthStencilSurface:   ProcSetDepthStencilSurface(cmd); break;
                case RenderCommandType::Clear:                    ProcClear(cmd); break;
                case RenderCommandType::SetViewport:              ProcSetViewport(cmd); break;
                case RenderCommandType::SetTexture:               ProcSetTexture(cmd); break;
                case RenderCommandType::SetScissorRect:           ProcSetScissorRect(cmd); break;
                case RenderCommandType::SetSamplerState:          ProcSetSamplerState(cmd); break;
                case RenderCommandType::SetBooleans:              ProcSetBooleans(cmd); break;
                case RenderCommandType::SetVertexShaderConstants: ProcSetVertexShaderConstants(cmd); break;
                case RenderCommandType::SetPixelShaderConstants:  ProcSetPixelShaderConstants(cmd); break;
                case RenderCommandType::AddPipeline:              ProcAddPipeline(cmd); break;
                case RenderCommandType::DrawPrimitive:            ProcDrawPrimitive(cmd); break;
                case RenderCommandType::DrawIndexedPrimitive:     ProcDrawIndexedPrimitive(cmd); break;
                case RenderCommandType::DrawPrimitiveUP:          ProcDrawPrimitiveUP(cmd); break;
                case RenderCommandType::SetVertexDeclaration:     ProcSetVertexDeclaration(cmd); break;
                case RenderCommandType::SetVertexShader:          ProcSetVertexShader(cmd); break;
                case RenderCommandType::SetStreamSource:          ProcSetStreamSource(cmd); break;
                case RenderCommandType::SetIndices:               ProcSetIndices(cmd); break;
                case RenderCommandType::SetPixelShader:           ProcSetPixelShader(cmd); break;
                default:                                          assert(false && "Unrecognized render command type."); break;
                }
            }
        }
    });

static void D3DXFillTexture(GuestTexture* texture, uint32_t function, void* data)
{
    if (texture->width == 1 && texture->height == 1 && texture->format == RenderFormat::R8_UNORM && function == 0x82BA2150)
    {
        auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(PLACEMENT_ALIGNMENT));

        uint8_t* mappedData = reinterpret_cast<uint8_t*>(uploadBuffer->map());
        *mappedData = 0xFF;
        uploadBuffer->unmap();

        ExecuteCopyCommandList([&]
            {
                g_copyCommandList->barriers(RenderBarrierStage::COPY, RenderTextureBarrier(texture->texture, RenderTextureLayout::COPY_DEST));

                g_copyCommandList->copyTextureRegion(
                    RenderTextureCopyLocation::Subresource(texture->texture, 0),
                    RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), texture->format, 1, 1, 1, PLACEMENT_ALIGNMENT, 0));
            });

        texture->layout = RenderTextureLayout::COPY_DEST;
    }
}

static void D3DXFillVolumeTexture(GuestTexture* texture, uint32_t function, void* data)
{
    uint32_t rowPitch0 = (texture->width * 4 + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
    uint32_t slicePitch0 = (rowPitch0 * texture->height * texture->depth + PLACEMENT_ALIGNMENT - 1) & ~(PLACEMENT_ALIGNMENT - 1);

    uint32_t rowPitch1 = ((texture->width / 2) * 4 + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
    uint32_t slicePitch1 = (rowPitch1 * (texture->height / 2) * (texture->depth / 2) + PLACEMENT_ALIGNMENT - 1) & ~(PLACEMENT_ALIGNMENT - 1);

    auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(slicePitch0 + slicePitch1));
    uint8_t* mappedData = reinterpret_cast<uint8_t*>(uploadBuffer->map());

    thread_local std::vector<float> mipData;
    mipData.resize((texture->width / 2) * (texture->height / 2) * (texture->depth / 2) * 4);
    memset(mipData.data(), 0, mipData.size() * sizeof(float));

    for (size_t z = 0; z < texture->depth; z++)
    {
        for (size_t y = 0; y < texture->height; y++)
        {
            for (size_t x = 0; x < texture->width; x++)
            {
                auto dest = mappedData + z * rowPitch0 * texture->height + y * rowPitch0 + x * sizeof(uint32_t);
                size_t index = z * texture->width * texture->height + y * texture->width + x;
                size_t mipIndex = ((z / 2) * (texture->width / 2) * (texture->height / 2) + (y / 2) * (texture->width / 2) + x / 2) * 4;

                if (function == 0x82BC7820)
                {
                    auto src = reinterpret_cast<be<float>*>(data) + index * 4;

                    float r = static_cast<uint8_t>(src[0] * 255.0f);
                    float g = static_cast<uint8_t>(src[1] * 255.0f);
                    float b = static_cast<uint8_t>(src[2] * 255.0f);
                    float a = static_cast<uint8_t>(src[3] * 255.0f);

                    dest[0] = r;
                    dest[1] = g;
                    dest[2] = b;
                    dest[3] = a;

                    mipData[mipIndex + 0] += r;
                    mipData[mipIndex + 1] += g;
                    mipData[mipIndex + 2] += b;
                    mipData[mipIndex + 3] += a;
                }
                else if (function == 0x82BC78A8)
                {
                    auto src = reinterpret_cast<uint8_t*>(data) + index * 4;

                    dest[0] = src[3];
                    dest[1] = src[2];
                    dest[2] = src[1];
                    dest[3] = src[0];

                    mipData[mipIndex + 0] += src[3];
                    mipData[mipIndex + 1] += src[2];
                    mipData[mipIndex + 2] += src[1];
                    mipData[mipIndex + 3] += src[0];
                }
            }
        }
    }

    for (size_t z = 0; z < texture->depth / 2; z++)
    {
        for (size_t y = 0; y < texture->height / 2; y++)
        {
            for (size_t x = 0; x < texture->width / 2; x++)
            {
                auto dest = mappedData + slicePitch0 + z * rowPitch1 * (texture->height / 2) + y * rowPitch1 + x * sizeof(uint32_t);
                size_t index = (z * (texture->width / 2) * (texture->height / 2) + y * (texture->width / 2) + x) * 4;

                dest[0] = static_cast<uint8_t>(mipData[index + 0] / 8.0f);
                dest[1] = static_cast<uint8_t>(mipData[index + 1] / 8.0f);
                dest[2] = static_cast<uint8_t>(mipData[index + 2] / 8.0f);
                dest[3] = static_cast<uint8_t>(mipData[index + 3] / 8.0f);
            }
        }
    }

    uploadBuffer->unmap();

    ExecuteCopyCommandList([&]
        {
            g_copyCommandList->barriers(RenderBarrierStage::COPY, RenderTextureBarrier(texture->texture, RenderTextureLayout::COPY_DEST));

            g_copyCommandList->copyTextureRegion(
                RenderTextureCopyLocation::Subresource(texture->texture, 0),
                RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), texture->format, texture->width, texture->height, texture->depth, rowPitch0 / RenderFormatSize(texture->format), 0));

            g_copyCommandList->copyTextureRegion(
                RenderTextureCopyLocation::Subresource(texture->texture, 1),
                RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), texture->format, texture->width / 2, texture->height / 2, texture->depth / 2, rowPitch1 / RenderFormatSize(texture->format), slicePitch0));
        });

    texture->layout = RenderTextureLayout::COPY_DEST;
}

struct GuestPictureData
{
    be<uint32_t> vtable;
    uint8_t flags;
    be<uint32_t> name;
    be<uint32_t> texture;
    be<uint32_t> type;
};

static RenderTextureDimension ConvertTextureDimension(ddspp::TextureType type)
{
    switch (type) 
    {
    case ddspp::Texture1D:
        return RenderTextureDimension::TEXTURE_1D;
    case ddspp::Texture2D:
    case ddspp::Cubemap:
        return RenderTextureDimension::TEXTURE_2D;
    case ddspp::Texture3D:
        return RenderTextureDimension::TEXTURE_3D;
    default:
        assert(false && "Unknown texture type from DDS.");
        return RenderTextureDimension::UNKNOWN;
    }
}

static RenderTextureViewDimension ConvertTextureViewDimension(ddspp::TextureType type)
{
    switch (type)
    {
    case ddspp::Texture1D:
        return RenderTextureViewDimension::TEXTURE_1D;
    case ddspp::Texture2D:
        return RenderTextureViewDimension::TEXTURE_2D;
    case ddspp::Texture3D:
        return RenderTextureViewDimension::TEXTURE_3D;
    case ddspp::Cubemap:
        return RenderTextureViewDimension::TEXTURE_CUBE;
    default:
        assert(false && "Unknown texture type from DDS.");
        return RenderTextureViewDimension::UNKNOWN;
    }
}

static RenderFormat ConvertDXGIFormat(ddspp::DXGIFormat format) 
{
    switch (format)
    {
    case ddspp::R32G32B32A32_TYPELESS:
        return RenderFormat::R32G32B32A32_TYPELESS;
    case ddspp::R32G32B32A32_FLOAT:
        return RenderFormat::R32G32B32A32_FLOAT;
    case ddspp::R32G32B32A32_UINT:
        return RenderFormat::R32G32B32A32_UINT;
    case ddspp::R32G32B32A32_SINT:
        return RenderFormat::R32G32B32A32_SINT;
    case ddspp::R32G32B32_TYPELESS:
        return RenderFormat::R32G32B32_TYPELESS;
    case ddspp::R32G32B32_FLOAT:
        return RenderFormat::R32G32B32_FLOAT;
    case ddspp::R32G32B32_UINT:
        return RenderFormat::R32G32B32_UINT;
    case ddspp::R32G32B32_SINT:
        return RenderFormat::R32G32B32_SINT;
    case ddspp::R16G16B16A16_TYPELESS:
        return RenderFormat::R16G16B16A16_TYPELESS;
    case ddspp::R16G16B16A16_FLOAT:
        return RenderFormat::R16G16B16A16_FLOAT;
    case ddspp::R16G16B16A16_UNORM:
        return RenderFormat::R16G16B16A16_UNORM;
    case ddspp::R16G16B16A16_UINT:
        return RenderFormat::R16G16B16A16_UINT;
    case ddspp::R16G16B16A16_SNORM:
        return RenderFormat::R16G16B16A16_SNORM;
    case ddspp::R16G16B16A16_SINT:
        return RenderFormat::R16G16B16A16_SINT;
    case ddspp::R32G32_TYPELESS:
        return RenderFormat::R32G32_TYPELESS;
    case ddspp::R32G32_FLOAT:
        return RenderFormat::R32G32_FLOAT;
    case ddspp::R32G32_UINT:
        return RenderFormat::R32G32_UINT;
    case ddspp::R32G32_SINT:
        return RenderFormat::R32G32_SINT;
    case ddspp::R8G8B8A8_TYPELESS:
        return RenderFormat::R8G8B8A8_TYPELESS;
    case ddspp::R8G8B8A8_UNORM:
        return RenderFormat::R8G8B8A8_UNORM;
    case ddspp::R8G8B8A8_UINT:
        return RenderFormat::R8G8B8A8_UINT;
    case ddspp::R8G8B8A8_SNORM:
        return RenderFormat::R8G8B8A8_SNORM;
    case ddspp::R8G8B8A8_SINT:
        return RenderFormat::R8G8B8A8_SINT;
    case ddspp::B8G8R8A8_UNORM:
        return RenderFormat::B8G8R8A8_UNORM;
    case ddspp::B8G8R8X8_UNORM:
        return RenderFormat::B8G8R8A8_UNORM;   
    case ddspp::R16G16_TYPELESS:
        return RenderFormat::R16G16_TYPELESS;
    case ddspp::R16G16_FLOAT:
        return RenderFormat::R16G16_FLOAT;
    case ddspp::R16G16_UNORM:
        return RenderFormat::R16G16_UNORM;
    case ddspp::R16G16_UINT:
        return RenderFormat::R16G16_UINT;
    case ddspp::R16G16_SNORM:
        return RenderFormat::R16G16_SNORM;
    case ddspp::R16G16_SINT:
        return RenderFormat::R16G16_SINT;
    case ddspp::R32_TYPELESS:
        return RenderFormat::R32_TYPELESS;
    case ddspp::D32_FLOAT:
        return RenderFormat::D32_FLOAT;
    case ddspp::R32_FLOAT:
        return RenderFormat::R32_FLOAT;
    case ddspp::R32_UINT:
        return RenderFormat::R32_UINT;
    case ddspp::R32_SINT:
        return RenderFormat::R32_SINT;
    case ddspp::R8G8_TYPELESS:
        return RenderFormat::R8G8_TYPELESS;
    case ddspp::R8G8_UNORM:
        return RenderFormat::R8G8_UNORM;
    case ddspp::R8G8_UINT:
        return RenderFormat::R8G8_UINT;
    case ddspp::R8G8_SNORM:
        return RenderFormat::R8G8_SNORM;
    case ddspp::R8G8_SINT:
        return RenderFormat::R8G8_SINT;
    case ddspp::R16_TYPELESS:
        return RenderFormat::R16_TYPELESS;
    case ddspp::R16_FLOAT:
        return RenderFormat::R16_FLOAT;
    case ddspp::D16_UNORM:
        return RenderFormat::D16_UNORM;
    case ddspp::R16_UNORM:
        return RenderFormat::R16_UNORM;
    case ddspp::R16_UINT:
        return RenderFormat::R16_UINT;
    case ddspp::R16_SNORM:
        return RenderFormat::R16_SNORM;
    case ddspp::R16_SINT:
        return RenderFormat::R16_SINT;
    case ddspp::R8_TYPELESS:
        return RenderFormat::R8_TYPELESS;
    case ddspp::R8_UNORM:
        return RenderFormat::R8_UNORM;
    case ddspp::R8_UINT:
        return RenderFormat::R8_UINT;
    case ddspp::R8_SNORM:
        return RenderFormat::R8_SNORM;
    case ddspp::R8_SINT:
        return RenderFormat::R8_SINT;
    case ddspp::BC1_TYPELESS:
        return RenderFormat::BC1_TYPELESS;
    case ddspp::BC1_UNORM:
        return RenderFormat::BC1_UNORM;
    case ddspp::BC1_UNORM_SRGB:
        return RenderFormat::BC1_UNORM_SRGB;
    case ddspp::BC2_TYPELESS:
        return RenderFormat::BC2_TYPELESS;
    case ddspp::BC2_UNORM:
        return RenderFormat::BC2_UNORM;
    case ddspp::BC2_UNORM_SRGB:
        return RenderFormat::BC2_UNORM_SRGB;
    case ddspp::BC3_TYPELESS:
        return RenderFormat::BC3_TYPELESS;
    case ddspp::BC3_UNORM:
        return RenderFormat::BC3_UNORM;
    case ddspp::BC3_UNORM_SRGB:
        return RenderFormat::BC3_UNORM_SRGB;
    case ddspp::BC4_TYPELESS:
        return RenderFormat::BC4_TYPELESS;
    case ddspp::BC4_UNORM:
        return RenderFormat::BC4_UNORM;
    case ddspp::BC4_SNORM:
        return RenderFormat::BC4_SNORM;
    case ddspp::BC5_TYPELESS:
        return RenderFormat::BC5_TYPELESS;
    case ddspp::BC5_UNORM:
        return RenderFormat::BC5_UNORM;
    case ddspp::BC5_SNORM:
        return RenderFormat::BC5_SNORM;
    case ddspp::BC6H_TYPELESS:
        return RenderFormat::BC6H_TYPELESS;
    case ddspp::BC6H_UF16:
        return RenderFormat::BC6H_UF16;
    case ddspp::BC6H_SF16:
        return RenderFormat::BC6H_SF16;
    case ddspp::BC7_TYPELESS:
        return RenderFormat::BC7_TYPELESS;
    case ddspp::BC7_UNORM:
        return RenderFormat::BC7_UNORM;
    case ddspp::BC7_UNORM_SRGB:
        return RenderFormat::BC7_UNORM_SRGB;
    default:
        assert(false && "Unsupported format from DDS.");
        return RenderFormat::UNKNOWN;
    }
}

static bool LoadTexture(GuestTexture& texture, const uint8_t* data, size_t dataSize, RenderComponentMapping componentMapping)
{
    ddspp::Descriptor ddsDesc;
    if (ddspp::decode_header((unsigned char *)(data), ddsDesc) != ddspp::Error)
    {
        RenderTextureDesc desc;
        desc.dimension = ConvertTextureDimension(ddsDesc.type);
        desc.width = ddsDesc.width;
        desc.height = ddsDesc.height;
        desc.depth = ddsDesc.depth;
        desc.mipLevels = ddsDesc.numMips;
        desc.arraySize = ddsDesc.type == ddspp::TextureType::Cubemap ? ddsDesc.arraySize * 6 : ddsDesc.arraySize;
        desc.format = ConvertDXGIFormat(ddsDesc.format);
        desc.flags = ddsDesc.type == ddspp::TextureType::Cubemap ? RenderTextureFlag::CUBE : RenderTextureFlag::NONE;

        texture.textureHolder = g_device->createTexture(desc);
        texture.texture = texture.textureHolder.get();
        texture.layout = RenderTextureLayout::COPY_DEST;

        RenderTextureViewDesc viewDesc;
        viewDesc.format = desc.format;
        viewDesc.dimension = ConvertTextureViewDimension(ddsDesc.type);
        viewDesc.mipLevels = ddsDesc.numMips;
        viewDesc.componentMapping = componentMapping;
        texture.textureView = texture.texture->createTextureView(viewDesc);
        texture.descriptorIndex = g_textureDescriptorAllocator.allocate();
        g_textureDescriptorSet->setTexture(texture.descriptorIndex, texture.texture, RenderTextureLayout::SHADER_READ, texture.textureView.get());

        texture.viewDimension = viewDesc.dimension;

        struct Slice
        {
            uint32_t width;
            uint32_t height;
            uint32_t depth;
            uint32_t srcOffset;
            uint32_t dstOffset;
            uint32_t srcRowPitch;
            uint32_t dstRowPitch;
            uint32_t rowCount;
        };

        std::vector<Slice> slices;
        uint32_t curSrcOffset = 0;
        uint32_t curDstOffset = 0;

        for (uint32_t arraySlice = 0; arraySlice < desc.arraySize; arraySlice++)
        {
            for (uint32_t mipSlice = 0; mipSlice < ddsDesc.numMips; mipSlice++)
            {
                auto& slice = slices.emplace_back();

                slice.width = std::max(1u, ddsDesc.width >> mipSlice);
                slice.height = std::max(1u, ddsDesc.height >> mipSlice);
                slice.depth = std::max(1u, ddsDesc.depth >> mipSlice);
                slice.srcOffset = curSrcOffset;
                slice.dstOffset = curDstOffset;
                uint32_t rowPitch = ((slice.width + ddsDesc.blockWidth - 1) / ddsDesc.blockWidth) * ddsDesc.bitsPerPixelOrBlock;
                slice.srcRowPitch = (rowPitch + 7) / 8;
                slice.dstRowPitch = (slice.srcRowPitch + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
                slice.rowCount = (slice.height + ddsDesc.blockHeight - 1) / ddsDesc.blockHeight;

                curSrcOffset += slice.srcRowPitch * slice.rowCount * slice.depth;
                curDstOffset += (slice.dstRowPitch * slice.rowCount * slice.depth + PLACEMENT_ALIGNMENT - 1) & ~(PLACEMENT_ALIGNMENT - 1);
            }
        }

        auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(curDstOffset));
        uint8_t* mappedMemory = reinterpret_cast<uint8_t*>(uploadBuffer->map());

        for (auto& slice : slices)
        {
            const uint8_t* srcData = data + ddsDesc.headerSize + slice.srcOffset;
            uint8_t* dstData = mappedMemory + slice.dstOffset;

            if (slice.srcRowPitch == slice.dstRowPitch)
            {
                memcpy(dstData, srcData, slice.srcRowPitch * slice.rowCount * slice.depth);
            }
            else
            {
                for (size_t i = 0; i < slice.rowCount * slice.depth; i++)
                {
                    memcpy(dstData, srcData, slice.srcRowPitch);
                    srcData += slice.srcRowPitch;
                    dstData += slice.dstRowPitch;
                }
            }
        }

        uploadBuffer->unmap();

        ExecuteCopyCommandList([&]
            {
                g_copyCommandList->barriers(RenderBarrierStage::COPY, RenderTextureBarrier(texture.texture, RenderTextureLayout::COPY_DEST));

                for (size_t i = 0; i < slices.size(); i++)
                {
                    auto& slice = slices[i];

                    g_copyCommandList->copyTextureRegion(
                        RenderTextureCopyLocation::Subresource(texture.texture, i),
                        RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), desc.format, slice.width, slice.height, slice.depth, (slice.dstRowPitch * 8) / ddsDesc.bitsPerPixelOrBlock * ddsDesc.blockWidth, slice.dstOffset));
                }
            });

        return true;
    }
    else
    {
        int width, height;
        void* stbImage = stbi_load_from_memory(data, dataSize, &width, &height, nullptr, 4);

        if (stbImage != nullptr)
        {
            texture.textureHolder = g_device->createTexture(RenderTextureDesc::Texture2D(width, height, 1, RenderFormat::R8G8B8A8_UNORM));
            texture.texture = texture.textureHolder.get();
            texture.viewDimension = RenderTextureViewDimension::TEXTURE_2D;
            texture.layout = RenderTextureLayout::COPY_DEST;

            texture.descriptorIndex = g_textureDescriptorAllocator.allocate();
            g_textureDescriptorSet->setTexture(texture.descriptorIndex, texture.texture, RenderTextureLayout::SHADER_READ);

            uint32_t rowPitch = (width * 4 + PITCH_ALIGNMENT - 1) & ~(PITCH_ALIGNMENT - 1);
            uint32_t slicePitch = rowPitch * height;

            auto uploadBuffer = g_device->createBuffer(RenderBufferDesc::UploadBuffer(slicePitch));
            uint8_t* mappedMemory = reinterpret_cast<uint8_t*>(uploadBuffer->map());

            if (rowPitch == (width * 4))
            {
                memcpy(mappedMemory, stbImage, slicePitch);
            }
            else
            {
                auto data = reinterpret_cast<const uint8_t*>(stbImage);

                for (size_t i = 0; i < height; i++)
                {
                    memcpy(mappedMemory, data, width * 4);
                    data += width * 4;
                    mappedMemory += rowPitch;
                }
            }

            uploadBuffer->unmap();

            stbi_image_free(stbImage);

            ExecuteCopyCommandList([&]
                {
                    g_copyCommandList->barriers(RenderBarrierStage::COPY, RenderTextureBarrier(texture.texture, RenderTextureLayout::COPY_DEST));

                    g_copyCommandList->copyTextureRegion(
                        RenderTextureCopyLocation::Subresource(texture.texture, 0),
                        RenderTextureCopyLocation::PlacedFootprint(uploadBuffer.get(), RenderFormat::R8G8B8A8_UNORM, width, height, 1, rowPitch / 4, 0));
                });

            return true;
        }
    }

    return false;
}

std::unique_ptr<GuestTexture> LoadTexture(const uint8_t* data, size_t dataSize, RenderComponentMapping componentMapping)
{
    GuestTexture texture(ResourceType::Texture);

    if (LoadTexture(texture, data, dataSize, componentMapping))
        return std::make_unique<GuestTexture>(std::move(texture));

    return nullptr;
}

static void DiffPatchTexture(GuestTexture& texture, uint8_t* data, uint32_t dataSize)
{
    auto header = reinterpret_cast<BlockCompressionDiffPatchHeader*>(g_buttonBcDiff.get());
    auto entries = reinterpret_cast<BlockCompressionDiffPatchEntry*>(g_buttonBcDiff.get() + header->entriesOffset);
    auto end = entries + header->entryCount;
    
    XXH64_hash_t hash = XXH3_64bits(data, dataSize);
    auto findResult = std::lower_bound(entries, end, hash, [](BlockCompressionDiffPatchEntry& lhs, XXH64_hash_t rhs)
        {
            return lhs.hash < rhs;
        });

    if (findResult != end && findResult->hash == hash)
    {
        auto patch = reinterpret_cast<BlockCompressionDiffPatch*>(g_buttonBcDiff.get() + findResult->patchesOffset);
        for (size_t i = 0; i < findResult->patchCount; i++)
        {
            assert(patch->destinationOffset + patch->patchBytesSize <= dataSize);
            memcpy(data + patch->destinationOffset, g_buttonBcDiff.get() + patch->patchBytesOffset, patch->patchBytesSize);
            ++patch;
        }

        GuestTexture patchedTexture(ResourceType::Texture);
        if (LoadTexture(patchedTexture, data, dataSize, {}))
            texture.patchedTexture = std::make_unique<GuestTexture>(std::move(patchedTexture));
    }
}

static void MakePictureData(GuestPictureData* pictureData, uint8_t* data, uint32_t dataSize)
{
    if ((pictureData->flags & 0x1) == 0 && data != nullptr)
    {
        GuestTexture texture(ResourceType::Texture);

        if (LoadTexture(texture, data, dataSize, {}))
        {
#ifdef _DEBUG
            texture.texture->setName(reinterpret_cast<char*>(g_memory.Translate(pictureData->name + 2)));
#endif

            DiffPatchTexture(texture, data, dataSize);

            pictureData->texture = g_memory.MapVirtual(g_userHeap.AllocPhysical<GuestTexture>(std::move(texture)));
            pictureData->type = 0;
        }
    }
}

void IndexBufferLengthMidAsmHook(PPCRegister& r3)
{
    r3.u64 *= 2;
}

void SetShadowResolutionMidAsmHook(PPCRegister& r11)
{
    auto res = (int32_t)Config::ShadowResolution.Value;

    if (res > 0)
        r11.u64 = res;
}

static void SetResolution(be<uint32_t>* device)
{
    uint32_t width = uint32_t(round(g_swapChain->getWidth() * Config::ResolutionScale));
    uint32_t height = uint32_t(round(g_swapChain->getHeight() * Config::ResolutionScale));
    device[46] = width == 0 ? 880 : width;
    device[47] = height == 0 ? 720 : height;
}

// The game does some weird stuff to render targets if they are above 
// 1024x1024 resolution, setting this bool at address 20 seems to avoid all that.
PPC_FUNC(sub_82E9F048)
{
    PPC_STORE_U8(ctx.r4.u32 + 20, 1);
    PPC_STORE_U32(ctx.r4.u32 + 44, PPC_LOAD_U32(ctx.r4.u32 + 8)); // Width
    PPC_STORE_U32(ctx.r4.u32 + 48, PPC_LOAD_U32(ctx.r4.u32 + 12)); // Height
}

static GuestShader* g_movieVertexShader;
static GuestShader* g_moviePixelShader;
static GuestVertexDeclaration* g_movieVertexDeclaration;

static void ScreenShaderInit(be<uint32_t>* a1, uint32_t a2, uint32_t a3, GuestVertexElement* vertexElements)
{
    if (g_moviePixelShader == nullptr)
    {
        g_moviePixelShader = g_userHeap.AllocPhysical<GuestShader>(ResourceType::PixelShader);
        g_moviePixelShader->shader = CREATE_SHADER(movie_ps);
    }

    if (g_movieVertexShader == nullptr)
    {
        g_movieVertexShader = g_userHeap.AllocPhysical<GuestShader>(ResourceType::VertexShader);
        g_movieVertexShader->shader = CREATE_SHADER(movie_vs);
    }

    if (g_movieVertexDeclaration == nullptr)
        g_movieVertexDeclaration = CreateVertexDeclarationWithoutAddRef(vertexElements);

    g_moviePixelShader->AddRef();
    g_movieVertexShader->AddRef();
    g_movieVertexDeclaration->AddRef();

    a1[2] = g_memory.MapVirtual(g_moviePixelShader);
    a1[3] = g_memory.MapVirtual(g_movieVertexShader);
    a1[4] = g_memory.MapVirtual(g_movieVertexDeclaration);
}

void MovieRendererMidAsmHook(PPCRegister& r3)
{
    auto device = reinterpret_cast<GuestDevice*>(g_memory.Translate(r3.u32));

    // Force linear filtering & clamp addressing
    for (size_t i = 0; i < 3; i++)
    {
        device->samplerStates[i].data[0] = (device->samplerStates[i].data[0].get() & ~0x7fc00) | 0x24800;
        device->samplerStates[i].data[3] = (device->samplerStates[i].data[3].get() & ~0x1f80000) | 0x1280000;
    }

    device->dirtyFlags[3] = device->dirtyFlags[3].get() | 0xe0000000ull;
}

static PPCRegister g_r4;
static PPCRegister g_r5;

// CRenderDirectorFxPipeline::Initialize
PPC_FUNC_IMPL(__imp__sub_8258C8A0);
PPC_FUNC(sub_8258C8A0)
{
    g_r4 = ctx.r4;
    g_r5 = ctx.r5;
    __imp__sub_8258C8A0(ctx, base);
}

// CRenderDirectorFxPipeline::Update
PPC_FUNC_IMPL(__imp__sub_8258CAE0);
PPC_FUNC(sub_8258CAE0)
{
    if (g_needsResize)
    {
        auto r3 = ctx.r3;
        ctx.r4 = g_r4;
        ctx.r5 = g_r5;
        __imp__sub_8258C8A0(ctx, base);
        ctx.r3 = r3;

        g_needsResize = false;
    }

    __imp__sub_8258CAE0(ctx, base);
}

void PostProcessResolutionFix(PPCRegister& r4, PPCRegister& f1, PPCRegister& f2)
{
    auto device = reinterpret_cast<be<uint32_t>*>(g_memory.Translate(r4.u32));

    uint32_t width = device[46].get();
    uint32_t height = device[47].get();

#if 0
    // TODO: Figure out why this breaks for height > weight
    double factor;
    if (width > height)
        factor = 720.0 / double(height);
    else
        factor = 1280.0 / double(width);
#else 
    double factor = 720.0 / double(height);
#endif

    f1.f64 *= factor;
    f2.f64 *= factor;
}

void LightShaftAspectRatioFix(PPCRegister& f28, PPCRegister& f0)
{
    f28.f64 = f0.f64;
}

static const be<uint16_t> g_particleTestIndexBuffer[] =
{
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 5
};

bool ParticleTestIndexBufferMidAsmHook(PPCRegister& r30)
{
    if (!g_triangleFanSupported)
    {
        auto buffer = CreateIndexBuffer(sizeof(g_particleTestIndexBuffer), 0, D3DFMT_INDEX16);
        void* memory = LockIndexBuffer(buffer, 0, 0, 0);
        memcpy(memory, g_particleTestIndexBuffer, sizeof(g_particleTestIndexBuffer));
        UnlockIndexBuffer(buffer);

        r30.u32 = g_memory.MapVirtual(buffer);
        return true;
    }
    return false;
}

void ParticleTestDrawIndexedPrimitiveMidAsmHook(PPCRegister& r7)
{
    if (!g_triangleFanSupported)
        r7.u64 = std::size(g_particleTestIndexBuffer);
}

void MotionBlurPrevInvViewProjectionMidAsmHook(PPCRegister& r10)
{
    auto mtxProjection = reinterpret_cast<be<float>*>(g_memory.Translate(r10.u32));

    // Reverse Z. Have to be done on CPU side because the matrix multiplications
    // add up and it loses precision by the time it's sent to GPU.
    mtxProjection[10] = -(mtxProjection[10] + 1.0f);
    mtxProjection[14] = -mtxProjection[14];
}

// Normally, we could delay setting IsMadeOne, but the game relies on that flag
// being present to handle load priority. To work around that, we can prevent
// IsMadeAll from being set until the compilation is finished. Time for a custom flag!
enum
{
    eDatabaseDataFlags_CompilingPipelines = 0x80
};

// This is passed to pipeline compilation threads to keep the loading screen busy until 
// all of them are finished. A shared pointer makes sure the destructor is called only once.
struct DatabaseDataHolder
{
    boost::shared_ptr<Hedgehog::Database::CDatabaseData> databaseData;

    DatabaseDataHolder() : databaseData()
    {
    }

    DatabaseDataHolder(const DatabaseDataHolder&) = delete;
    DatabaseDataHolder(DatabaseDataHolder&& other)
        : databaseData(std::exchange(other.databaseData, nullptr))
    {
    }

    ~DatabaseDataHolder()
    {
        if (databaseData.get() != nullptr)
        {
            databaseData->m_Flags &= ~eDatabaseDataFlags_CompilingPipelines;

            if ((--g_compilingDataCount) == 0)
                g_compilingDataCount.notify_all();
        }
    }
};

struct PipelineStateQueueItem
{
    XXH64_hash_t pipelineHash;
    PipelineState pipelineState;
    std::shared_ptr<DatabaseDataHolder> databaseDataHolder;
#ifdef ASYNC_PSO_DEBUG
    std::string pipelineName;
#endif
};

static moodycamel::BlockingConcurrentQueue<PipelineStateQueueItem> g_pipelineStateQueue;

struct MinimalGuestThreadContext
{
    uint8_t* stack = nullptr;
    PPCContext ppcContext{};

    ~MinimalGuestThreadContext()
    {
        if (stack != nullptr)
            g_userHeap.Free(stack);
    }

    void ensureValid()
    {
        if (stack == nullptr)
        {
            stack = reinterpret_cast<uint8_t*>(g_userHeap.Alloc(0x4000));
            ppcContext.fn = (uint8_t*)g_codeCache.bucket;
            ppcContext.r1.u64 = g_memory.MapVirtual(stack + 0x4000);
            SetPPCContext(ppcContext);
        }
    }
};

static void PipelineCompilerThread()
{
    GuestThread::SetThreadName(GetCurrentThreadId(), "Pipeline Compiler Thread");
    MinimalGuestThreadContext ctx;

    while (true)
    {
        PipelineStateQueueItem queueItem;
        g_pipelineStateQueue.wait_dequeue(queueItem);

        ctx.ensureValid();

        auto pipeline = CreateGraphicsPipeline(queueItem.pipelineState);
#ifdef ASYNC_PSO_DEBUG
        pipeline->setName(queueItem.pipelineName);
#endif

        // Will get dropped in render thread if a different thread already managed to compile this.
        RenderCommand cmd;
        cmd.type = RenderCommandType::AddPipeline;
        cmd.addPipeline.hash = queueItem.pipelineHash;
        cmd.addPipeline.pipeline = pipeline.release();
        g_renderQueue.enqueue(cmd);
    }
}

static std::vector<std::unique_ptr<std::thread>> g_pipelineCompilerThreads = []()
    {
        size_t threadCount = std::max(2u, (std::thread::hardware_concurrency() * 2) / 3);

        std::vector<std::unique_ptr<std::thread>> threads(threadCount);
        for (auto& thread : threads)
            thread = std::make_unique<std::thread>(PipelineCompilerThread);

        return threads;
    }();

static constexpr uint32_t MODEL_DATA_VFTABLE = 0x82073A44;
static constexpr uint32_t TERRAIN_MODEL_DATA_VFTABLE = 0x8211D25C;
static constexpr uint32_t PARTICLE_MATERIAL_VFTABLE = 0x8211F198;

// Allocate the shared pointer only when new compilations are happening.
// If nothing was compiled, the local "holder" variable will get destructed with RAII instead.
struct DatabaseDataHolderPair
{
    DatabaseDataHolder holder;
    std::shared_ptr<DatabaseDataHolder> counter;
};

// Having this separate, because I don't want to lock a mutex in the render thread before
// every single draw. Might be worth profiling to see if it actually has an impact and merge them.
static ankerl::unordered_dense::set<XXH64_hash_t, xxHash> g_asyncPipelines;

static void EnqueueGraphicsPipelineCompilation(const PipelineState& pipelineState, DatabaseDataHolderPair& databaseDataHolderPair, const char* name)
{
    XXH64_hash_t hash = XXH3_64bits(&pipelineState, sizeof(pipelineState));
    bool shouldCompile = g_asyncPipelines.emplace(hash).second;

    if (shouldCompile)
    {
        if (databaseDataHolderPair.counter == nullptr && databaseDataHolderPair.holder.databaseData.get() != nullptr)
            databaseDataHolderPair.counter = std::make_shared<DatabaseDataHolder>(std::move(databaseDataHolderPair.holder));

        PipelineStateQueueItem queueItem;
        queueItem.pipelineHash = hash;
        queueItem.pipelineState = pipelineState;
        queueItem.databaseDataHolder = databaseDataHolderPair.counter;
#ifdef ASYNC_PSO_DEBUG
        queueItem.pipelineName = std::format("ASYNC {} {:X}", name, hash);
#endif
        g_pipelineStateQueue.enqueue(queueItem);
    }
}

struct CompilationArgs
{
    DatabaseDataHolderPair holderPair;
    bool noGI{};
    bool hasMoreThanOneBone{};
    bool velocityMapQuickStep{};
    bool objectIcon{};
};

enum class MeshLayer
{
    Opaque,
    Transparent,
    PunchThrough,
    Special
};

static void CompileMeshPipeline(Hedgehog::Mirage::CMeshData* mesh, MeshLayer layer, CompilationArgs& args)
{
    if (mesh->m_spMaterial.get() == nullptr || mesh->m_spMaterial->m_spShaderListData.get() == nullptr)
        return;

    auto& material = mesh->m_spMaterial;
    auto& shaderList = material->m_spShaderListData;

    bool isFur = strstr(shaderList->m_TypeAndName.c_str(), "Fur") != nullptr;
    bool isSky = strstr(shaderList->m_TypeAndName.c_str(), "Sky") != nullptr;
    bool isSonicMouth = strcmp(material->m_TypeAndName.c_str() + 2, "sonic_gm_mouth_duble") == 0 &&
        strcmp(shaderList->m_TypeAndName.c_str() + 3, "SonicSkin_dspf[b]") == 0;

    bool compiledOutsideMainFramebuffer = !isFur && !isSky;

    bool constTexCoord = true;
    if (material->m_spTexsetData.get() != nullptr)
    {
        for (size_t i = 1; i < material->m_spTexsetData->m_TextureList.size(); i++)
        {
            if (material->m_spTexsetData->m_TextureList[i]->m_TexcoordIndex !=
                material->m_spTexsetData->m_TextureList[0]->m_TexcoordIndex)
            {
                constTexCoord = false;
                break;
            }
        }
    }

    auto vertexDeclaration = reinterpret_cast<GuestVertexDeclaration*>(mesh->m_VertexDeclarationPtr.m_pD3DVertexDeclaration.get());

    // Shadow pipeline.
    if (compiledOutsideMainFramebuffer && (layer == MeshLayer::Opaque || layer == MeshLayer::PunchThrough))
    {
        PipelineState pipelineState{};

        if (layer == MeshLayer::PunchThrough)
        {
            pipelineState.vertexShader = FindShaderCacheEntry(0xDD4FA7BB53876300)->guestShader;
            pipelineState.pixelShader = FindShaderCacheEntry(0xE2ECA594590DDE8B)->guestShader;
        }
        else
        {
            pipelineState.vertexShader = FindShaderCacheEntry(0x8E4BB23465BD909E)->guestShader;
        }

        pipelineState.vertexDeclaration = vertexDeclaration;
        pipelineState.cullMode = material->m_DoubleSided ? RenderCullMode::NONE : RenderCullMode::BACK;
        pipelineState.zFunc = RenderComparisonFunction::LESS_EQUAL;
        pipelineState.depthBias = (1 << 24) * (*reinterpret_cast<be<float>*>(g_memory.Translate(0x83302760)));
        pipelineState.slopeScaledDepthBias = *reinterpret_cast<be<float>*>(g_memory.Translate(0x83302764));
        pipelineState.colorWriteEnable = 0;
        pipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_STRIP;
        pipelineState.vertexStrides[0] = mesh->m_VertexSize;
        pipelineState.depthStencilFormat = RenderFormat::D32_FLOAT;

        if (layer == MeshLayer::PunchThrough)
            pipelineState.specConstants |= SPEC_CONSTANT_ALPHA_TEST;

        SanitizePipelineState(pipelineState);
        EnqueueGraphicsPipelineCompilation(pipelineState, args.holderPair, layer == MeshLayer::PunchThrough ? "MakeShadowMapTransparent" : "MakeShadowMap");
    }

    // Motion blur pipeline. We could normally do the player here only, but apparently Werehog enemies also have object blur.
    // TODO: Do punch through meshes get rendered?
    if (compiledOutsideMainFramebuffer && args.hasMoreThanOneBone && layer == MeshLayer::Opaque)
    {
        PipelineState pipelineState{};
        pipelineState.vertexShader = FindShaderCacheEntry(0x4620B236DC38100C)->guestShader;
        pipelineState.pixelShader = FindShaderCacheEntry(0xBBDB735BEACC8F41)->guestShader;
        pipelineState.vertexDeclaration = vertexDeclaration;
        pipelineState.cullMode = RenderCullMode::NONE;
        pipelineState.zFunc = RenderComparisonFunction::GREATER_EQUAL;
        pipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_STRIP;
        pipelineState.vertexStrides[0] = mesh->m_VertexSize;
        pipelineState.renderTargetFormat = RenderFormat::R8G8B8A8_UNORM;
        pipelineState.depthStencilFormat = RenderFormat::D32_FLOAT;
        pipelineState.specConstants = SPEC_CONSTANT_REVERSE_Z;

        SanitizePipelineState(pipelineState);
        EnqueueGraphicsPipelineCompilation(pipelineState, args.holderPair, "FxVelocityMap");

        if (args.velocityMapQuickStep)
        {
            pipelineState.vertexShader = FindShaderCacheEntry(0x99DC3F27E402700D)->guestShader;
            SanitizePipelineState(pipelineState);
            EnqueueGraphicsPipelineCompilation(pipelineState, args.holderPair, "FxVelocityMapQuickStep");
        }
    }

    guest_stack_var<Hedgehog::Base::CStringSymbol> defaultSymbol(reinterpret_cast<const char*>(g_memory.Translate(0x8202DDBC)));
    auto defaultFindResult = shaderList->m_PixelShaderPermutations.find(*defaultSymbol);
    if (defaultFindResult == shaderList->m_PixelShaderPermutations.end())
        return;

    uint32_t pixelShaderSubPermutationsToCompile = 0;
    if (constTexCoord) pixelShaderSubPermutationsToCompile |= 0x1;
    if (args.noGI) pixelShaderSubPermutationsToCompile |= 0x2;

    if ((defaultFindResult->second.m_SubPermutations.get() & (1 << pixelShaderSubPermutationsToCompile)) == 0) pixelShaderSubPermutationsToCompile &= ~0x1;
    if ((defaultFindResult->second.m_SubPermutations.get() & (1 << pixelShaderSubPermutationsToCompile)) == 0) pixelShaderSubPermutationsToCompile &= ~0x2;

    guest_stack_var<Hedgehog::Base::CStringSymbol> noneSymbol(reinterpret_cast<const char*>(g_memory.Translate(0x8200D938)));
    auto noneFindResult = defaultFindResult->second.m_VertexShaderPermutations.find(*noneSymbol);
    if (noneFindResult == defaultFindResult->second.m_VertexShaderPermutations.end())
        return;

    uint32_t vertexShaderSubPermutationsToCompile = 0;
    if (constTexCoord) vertexShaderSubPermutationsToCompile |= 0x1;

    if ((noneFindResult->second->m_SubPermutations.get() & (1 << vertexShaderSubPermutationsToCompile)) == 0)
        vertexShaderSubPermutationsToCompile &= ~0x1;

    // Fur requires an instanced variant of the vertex declaration.
    if (isFur)
    {
        GuestVertexElement vertexElements[64];
        memcpy(vertexElements, vertexDeclaration->vertexElements.get(), (vertexDeclaration->vertexElementCount - 1) * sizeof(GuestVertexElement));

        vertexElements[vertexDeclaration->vertexElementCount - 1] = { 1, 0, 0x2C82A1, 0, 0, 1 };
        vertexElements[vertexDeclaration->vertexElementCount] = { 2, 0, 0x2C83A4, 0, 0, 2 };
        vertexElements[vertexDeclaration->vertexElementCount + 1] = D3DDECL_END();

        vertexDeclaration = CreateVertexDeclarationWithoutAddRef(vertexElements);
    }

    for (auto& [pixelShaderSubPermutations, pixelShader] : defaultFindResult->second.m_PixelShaders)
    {
        if (pixelShader.get() == nullptr || (pixelShaderSubPermutations & 0x3) != pixelShaderSubPermutationsToCompile)
            continue;

        for (auto& [vertexShaderSubPermutations, vertexShader] : noneFindResult->second->m_VertexShaders)
        {
            if (vertexShader.get() == nullptr || (vertexShaderSubPermutations & 0x1) != vertexShaderSubPermutationsToCompile)
                continue;

            PipelineState pipelineState{};
            pipelineState.vertexShader = reinterpret_cast<GuestShader*>(vertexShader->m_spCode->m_pD3DVertexShader.get());
            pipelineState.pixelShader = reinterpret_cast<GuestShader*>(pixelShader->m_spCode->m_pD3DPixelShader.get());
            pipelineState.vertexDeclaration = vertexDeclaration;
            pipelineState.instancing = isFur;
            pipelineState.zWriteEnable = !isSky && layer != MeshLayer::Transparent;
            pipelineState.srcBlend = RenderBlend::SRC_ALPHA;
            pipelineState.destBlend = material->m_Additive ? RenderBlend::ONE : RenderBlend::INV_SRC_ALPHA;
            pipelineState.cullMode = material->m_DoubleSided ? RenderCullMode::NONE : RenderCullMode::BACK;
            pipelineState.zFunc = RenderComparisonFunction::GREATER_EQUAL; // Reverse Z
            pipelineState.alphaBlendEnable = layer == MeshLayer::Transparent || layer == MeshLayer::Special;
            pipelineState.srcBlendAlpha = RenderBlend::SRC_ALPHA;
            pipelineState.destBlendAlpha = RenderBlend::INV_SRC_ALPHA;
            pipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_STRIP;
            pipelineState.vertexStrides[0] = mesh->m_VertexSize;
            pipelineState.vertexStrides[1] = isFur ? 4 : 0;
            pipelineState.vertexStrides[2] = isFur ? 4 : 0;
            pipelineState.renderTargetFormat = RenderFormat::R16G16B16A16_FLOAT;
            pipelineState.depthStencilFormat = RenderFormat::D32_FLOAT;
            pipelineState.sampleCount = Config::AntiAliasing != EAntiAliasing::None ? int32_t(Config::AntiAliasing.Value) : 1;

            if (pipelineState.vertexDeclaration->hasR11G11B10Normal)
                pipelineState.specConstants |= SPEC_CONSTANT_R11G11B10_NORMAL;

            if (Config::GITextureFiltering == EGITextureFiltering::Bicubic)
                pipelineState.specConstants |= SPEC_CONSTANT_BICUBIC_GI_FILTER;

            if (layer == MeshLayer::PunchThrough)
            {
                if (Config::AntiAliasing != EAntiAliasing::None && Config::TransparencyAntiAliasing)
                {
                    pipelineState.enableAlphaToCoverage = true;
                    pipelineState.specConstants |= SPEC_CONSTANT_ALPHA_TO_COVERAGE;
                }
                else
                {
                    pipelineState.specConstants |= SPEC_CONSTANT_ALPHA_TEST;
                }
            }

            if (!isSky)
                pipelineState.specConstants |= SPEC_CONSTANT_REVERSE_Z;

            auto createGraphicsPipeline = [&](PipelineState& pipelineStateToCreate)
                {
                    SanitizePipelineState(pipelineStateToCreate);
                    EnqueueGraphicsPipelineCompilation(pipelineStateToCreate, args.holderPair, shaderList->m_TypeAndName.c_str() + 3);
                };

            createGraphicsPipeline(pipelineState);

            bool planarReflectionEnabled = reinterpret_cast<bool*>(g_memory.Translate(0x832FA0D8));

            auto noMsaaPipeline = pipelineState;
            noMsaaPipeline.sampleCount = 1;
            noMsaaPipeline.enableAlphaToCoverage = false;

            if ((noMsaaPipeline.specConstants & SPEC_CONSTANT_ALPHA_TO_COVERAGE) != 0)
            {
                noMsaaPipeline.specConstants &= ~SPEC_CONSTANT_ALPHA_TO_COVERAGE;
                noMsaaPipeline.specConstants |= SPEC_CONSTANT_ALPHA_TEST;
            }

            if (planarReflectionEnabled)
            {
                // Planar reflections don't use MSAA.
                createGraphicsPipeline(noMsaaPipeline);
            }

            if (args.objectIcon) 
            {
                // Object icons get rendered to a SDR buffer without MSAA.
                auto iconPipelineState = noMsaaPipeline;
                iconPipelineState.renderTargetFormat = RenderFormat::R8G8B8A8_UNORM;
                createGraphicsPipeline(iconPipelineState);
            }

            if (isSonicMouth)
            {
                // Sonic's mouth switches between "SonicSkin_dspf[b]" or "SonicSkinNodeInvX_dspf[b]" depending on the view angle.
                auto mouthPipelineState = pipelineState;
                mouthPipelineState.vertexShader = FindShaderCacheEntry(0x689AA3140AB9EBAA)->guestShader;
                createGraphicsPipeline(mouthPipelineState);

                if (planarReflectionEnabled)
                {
                    auto noMsaaMouthPipelineState = noMsaaPipeline;
                    noMsaaMouthPipelineState.vertexShader = mouthPipelineState.vertexShader;
                    createGraphicsPipeline(noMsaaMouthPipelineState);
                }
            }
        }
    }
}

template<typename T>
static void CompileMeshPipelines(const T& modelData, CompilationArgs& args)
{
    for (auto& meshGroup : modelData.m_NodeGroupModels)
    {
        for (auto& mesh : meshGroup->m_OpaqueMeshes)
        {
            CompileMeshPipeline(mesh.get(), MeshLayer::Opaque, args);

            if (args.noGI) // For models that can be shown transparent (eg. medals)
                CompileMeshPipeline(mesh.get(), MeshLayer::Transparent, args);
        }

        for (auto& mesh : meshGroup->m_TransparentMeshes)
            CompileMeshPipeline(mesh.get(), MeshLayer::Transparent, args);

        for (auto& mesh : meshGroup->m_PunchThroughMeshes)
            CompileMeshPipeline(mesh.get(), MeshLayer::PunchThrough, args);

        for (auto& specialMeshGroup : meshGroup->m_SpecialMeshGroups)
        {
            for (auto& mesh : specialMeshGroup)
                CompileMeshPipeline(mesh.get(), MeshLayer::Special, args); // TODO: Are there layer types other than water in this game??
        }
    }

    for (auto& mesh : modelData.m_OpaqueMeshes)
    {
        CompileMeshPipeline(mesh.get(), MeshLayer::Opaque, args);

        if (args.noGI)
            CompileMeshPipeline(mesh.get(), MeshLayer::Transparent, args);
    }

    for (auto& mesh : modelData.m_TransparentMeshes)
        CompileMeshPipeline(mesh.get(), MeshLayer::Transparent, args);

    for (auto& mesh : modelData.m_PunchThroughMeshes)
        CompileMeshPipeline(mesh.get(), MeshLayer::PunchThrough, args);
}

static void CompileParticleMaterialPipeline(const Hedgehog::Sparkle::CParticleMaterial& material, DatabaseDataHolderPair& holderPair)
{
    auto& shaderList = material.m_spShaderListData;
    if (shaderList.get() == nullptr)
        return;

    guest_stack_var<Hedgehog::Base::CStringSymbol> defaultSymbol(reinterpret_cast<const char*>(g_memory.Translate(0x8202DDBC)));
    auto defaultFindResult = shaderList->m_PixelShaderPermutations.find(*defaultSymbol);
    if (defaultFindResult == shaderList->m_PixelShaderPermutations.end())
        return;

    guest_stack_var<Hedgehog::Base::CStringSymbol> noneSymbol(reinterpret_cast<const char*>(g_memory.Translate(0x8200D938)));
    auto noneFindResult = defaultFindResult->second.m_VertexShaderPermutations.find(*noneSymbol);
    if (noneFindResult == defaultFindResult->second.m_VertexShaderPermutations.end())
        return;

    // All the particle models in the game come with the unoptimized format, so we can assume it.
    uint8_t unoptimizedVertexElements[144] = 
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x23, 0xB9, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x0C, 0x00, 0x2A, 0x23, 0xB9, 0x00, 0x03, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x18, 0x00, 0x2A, 0x23, 0xB9, 0x00, 0x06, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x24, 0x00, 0x2A, 0x23, 0xB9, 0x00, 0x07, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x30, 0x00, 0x2C, 0x23, 0xA5, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x38, 0x00, 0x2C, 0x23, 0xA5, 0x00, 0x05, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x40, 0x00, 0x2C, 0x23, 0xA5, 0x00, 0x05, 0x02, 0x00,
        0x00, 0x00, 0x00, 0x48, 0x00, 0x2C, 0x23, 0xA5, 0x00, 0x05, 0x03, 0x00,
        0x00, 0x00, 0x00, 0x50, 0x00, 0x1A, 0x23, 0xA6, 0x00, 0x0A, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x60, 0x00, 0x1A, 0x23, 0x86, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x64, 0x00, 0x1A, 0x20, 0x86, 0x00, 0x01, 0x00, 0x00,
        0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00
    };

    auto unoptimizedVertexDeclaration = CreateVertexDeclarationWithoutAddRef(reinterpret_cast<GuestVertexElement*>(unoptimizedVertexElements));
    auto sparkleVertexDeclaration = CreateVertexDeclarationWithoutAddRef(reinterpret_cast<GuestVertexElement*>(g_memory.Translate(0x8211F540)));

    bool isMeshShader = strstr(shaderList->m_TypeAndName.c_str(), "Mesh") != nullptr;

    PipelineState pipelineState{};
    pipelineState.vertexShader = reinterpret_cast<GuestShader*>(noneFindResult->second->m_VertexShaders.begin()->second->m_spCode->m_pD3DVertexShader.get());
    pipelineState.pixelShader = reinterpret_cast<GuestShader*>(defaultFindResult->second.m_PixelShaders.begin()->second->m_spCode->m_pD3DPixelShader.get());
    pipelineState.vertexDeclaration = isMeshShader ? unoptimizedVertexDeclaration : sparkleVertexDeclaration;
    pipelineState.zWriteEnable = false;
    pipelineState.srcBlend = RenderBlend::SRC_ALPHA;
    pipelineState.destBlend = RenderBlend::INV_SRC_ALPHA;
    pipelineState.zFunc = RenderComparisonFunction::GREATER_EQUAL;
    pipelineState.alphaBlendEnable = true;
    pipelineState.srcBlendAlpha = RenderBlend::SRC_ALPHA;
    pipelineState.destBlendAlpha = RenderBlend::INV_SRC_ALPHA;
    pipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_STRIP;
    pipelineState.vertexStrides[0] = isMeshShader ? 104 : 28;
    pipelineState.renderTargetFormat = RenderFormat::R16G16B16A16_FLOAT;
    pipelineState.depthStencilFormat = RenderFormat::D32_FLOAT;
    pipelineState.sampleCount = Config::AntiAliasing != EAntiAliasing::None ? int32_t(Config::AntiAliasing.Value) : 1;
    pipelineState.specConstants = SPEC_CONSTANT_REVERSE_Z;

    if (pipelineState.vertexDeclaration->hasR11G11B10Normal)
        pipelineState.specConstants |= SPEC_CONSTANT_R11G11B10_NORMAL;

    switch (material.m_BlendMode.get())
    {
    case Hedgehog::Sparkle::CParticleMaterial::eBlendMode_Zero:
        // TODO: What are the render states for this??
        break;
    case Hedgehog::Sparkle::CParticleMaterial::eBlendMode_Typical:
        // Leave default.
        break;
    case Hedgehog::Sparkle::CParticleMaterial::eBlendMode_Add:
        pipelineState.destBlend = RenderBlend::ONE;
        break;
    case Hedgehog::Sparkle::CParticleMaterial::eBlendMode_Subtract:
        // TODO: Is this correct?
        pipelineState.destBlend = RenderBlend::ONE;
        pipelineState.blendOp = RenderBlendOperation::SUBTRACT;
        break;
    }

    auto createGraphicsPipeline = [&](PipelineState& pipelineStateToCreate)
        {
            SanitizePipelineState(pipelineStateToCreate);
            EnqueueGraphicsPipelineCompilation(pipelineStateToCreate, holderPair, shaderList->m_TypeAndName.c_str() + 3);
        };

    // TODO: See if this is necessary for everything.
    RenderCullMode cullModes[] = { RenderCullMode::NONE, RenderCullMode::BACK };

    for (auto cullMode : cullModes)
    {
        pipelineState.cullMode = cullMode;
        createGraphicsPipeline(pipelineState);

        bool planarReflectionEnabled = reinterpret_cast<bool*>(g_memory.Translate(0x832FA0D8));

        auto noMsaaPipelineState = pipelineState;
        noMsaaPipelineState.sampleCount = 1;

        if (planarReflectionEnabled)
            createGraphicsPipeline(noMsaaPipelineState);

        if (!isMeshShader)
        {
            // Previous compilation was for locus particles. This one will be for quads.
            auto quadPipelineState = pipelineState;
            quadPipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_LIST;
            createGraphicsPipeline(quadPipelineState);

            if (planarReflectionEnabled)
            {
                auto noMsaaQuadPipelineState = noMsaaPipelineState;
                noMsaaQuadPipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_LIST;
                createGraphicsPipeline(noMsaaQuadPipelineState);
            }
        }
    }
}

// SWA::CGameModeStage::ExitLoading
PPC_FUNC_IMPL(__imp__sub_825369A0);
PPC_FUNC(sub_825369A0)
{
    // Wait for pipeline compilations to finish.
    uint32_t value;
    while ((value = g_compilingDataCount.load()) != 0)
        g_compilingDataCount.wait(value);

    __imp__sub_825369A0(ctx, base);
}

// CModelData::CheckMadeAll
PPC_FUNC_IMPL(__imp__sub_82E2EFB0);
PPC_FUNC(sub_82E2EFB0)
{   
    if (reinterpret_cast<Hedgehog::Database::CDatabaseData*>(base + ctx.r3.u32)->m_Flags & eDatabaseDataFlags_CompilingPipelines)
    {
        ctx.r3.u64 = 0;
    }
    else
    {
        __imp__sub_82E2EFB0(ctx, base);
    }
}

// CTerrainModelData::CheckMadeAll
PPC_FUNC_IMPL(__imp__sub_82E243D8);
PPC_FUNC(sub_82E243D8)
{   
    if (reinterpret_cast<Hedgehog::Database::CDatabaseData*>(base + ctx.r3.u32)->m_Flags & eDatabaseDataFlags_CompilingPipelines)
    {
        ctx.r3.u64 = 0;
    }
    else
    {
        __imp__sub_82E243D8(ctx, base);
    }
}

// CParticleMaterial::CheckMadeAll
PPC_FUNC_IMPL(__imp__sub_82E87598);
PPC_FUNC(sub_82E87598)
{   
    if (reinterpret_cast<Hedgehog::Database::CDatabaseData*>(base + ctx.r3.u32)->m_Flags & eDatabaseDataFlags_CompilingPipelines)
    {
        ctx.r3.u64 = 0;
    }
    else
    {
        __imp__sub_82E87598(ctx, base);
    }
}

static Mutex g_pendingModelMutex;
static std::vector<boost::shared_ptr<Hedgehog::Database::CDatabaseData>> g_pendingDataQueue;

void GetDatabaseDataMidAsmHook(PPCRegister& r1, PPCRegister& r4)
{
    auto& databaseData = *reinterpret_cast<boost::shared_ptr<Hedgehog::Database::CDatabaseData>*>(
        g_memory.Translate(r1.u32 + 0x58));

    if (!databaseData->IsMadeOne() && r4.u32 != NULL)
    {
        if (databaseData->m_pVftable.ptr == MODEL_DATA_VFTABLE)
        {
            // Ignore particle models, the materials they point at don't actually
            // get used and give the threads unnecessary work.
            bool isParticleModel = *reinterpret_cast<be<uint32_t>*>(g_memory.Translate(r4.u32 + 4)) != 5 &&
                strncmp(databaseData->m_TypeAndName.c_str() + 2, "eff_", 4) == 0;

            if (isParticleModel)
                return;
        }

        ++g_compilingDataCount;
        databaseData->m_Flags |= eDatabaseDataFlags_CompilingPipelines;

        {
            std::lock_guard lock(g_pendingModelMutex);
            g_pendingDataQueue.push_back(databaseData);
        }

        if ((++g_pendingDataCount) == 1)
            g_pendingDataCount.notify_all();
    }
}

static bool CheckMadeAll(Hedgehog::Mirage::CMeshData* meshData)
{
    if (!meshData->IsMadeOne())
        return false;

    if (meshData->m_spMaterial.get() != nullptr)
    {
        if (!meshData->m_spMaterial->IsMadeOne())
            return false;

        if (meshData->m_spMaterial->m_spTexsetData.get() != nullptr)
        {
            if (!meshData->m_spMaterial->m_spTexsetData->IsMadeOne())
                return false;

            for (auto& texture : meshData->m_spMaterial->m_spTexsetData->m_TextureList)
            {
                if (!texture->IsMadeOne())
                    return false;
            }
        }
    }

    return true;
}

template<typename T>
static bool CheckMadeAll(const T& modelData)
{
    if (!modelData.IsMadeOne())
        return false;

    for (auto& meshGroup : modelData.m_NodeGroupModels)
    {
        for (auto& mesh : meshGroup->m_OpaqueMeshes)
        {
            if (!CheckMadeAll(mesh.get()))
                return false;
        }     

        for (auto& mesh : meshGroup->m_TransparentMeshes)
        {
            if (!CheckMadeAll(mesh.get()))
                return false;
        }    

        for (auto& mesh : meshGroup->m_PunchThroughMeshes)
        {
            if (!CheckMadeAll(mesh.get()))
                return false;
        }

        for (auto& specialMeshGroup : meshGroup->m_SpecialMeshGroups)
        {
            for (auto& mesh : specialMeshGroup)
            {
                if (!CheckMadeAll(mesh.get()))
                    return false;
            }
        }
    }

    for (auto& mesh : modelData.m_OpaqueMeshes)
    {
        if (!CheckMadeAll(mesh.get()))
            return false;
    }

    for (auto& mesh : modelData.m_TransparentMeshes)
    {
        if (!CheckMadeAll(mesh.get()))
            return false;
    }

    for (auto& mesh : modelData.m_PunchThroughMeshes)
    {
        if (!CheckMadeAll(mesh.get()))
            return false;
    }

    return true;
}

static void ModelConsumerThread()
{
    GuestThread::SetThreadName(GetCurrentThreadId(), "Model Consumer Thread");

    std::vector<boost::shared_ptr<Hedgehog::Database::CDatabaseData>> localPendingDataQueue;
    MinimalGuestThreadContext ctx;

    while (true)
    {
        // Wait for models to arrive.
        uint32_t pendingDataCount;
        while ((pendingDataCount = g_pendingDataCount.load()) == 0)
            g_pendingDataCount.wait(pendingDataCount);

        ctx.ensureValid();

        if (g_pendingPipelineStateCache)
        {
            DatabaseDataHolderPair emptyHolderPair;

            for (auto vertexElements : g_vertexDeclarationCache)
                CreateVertexDeclarationWithoutAddRef(reinterpret_cast<GuestVertexElement*>(vertexElements));

            for (auto pipelineState : g_pipelineStateCache)
            {
                // The hashes were reinterpret casted to pointers in the cache.
                pipelineState.vertexShader = FindShaderCacheEntry(reinterpret_cast<XXH64_hash_t>(pipelineState.vertexShader))->guestShader;

                if (pipelineState.pixelShader != nullptr)
                    pipelineState.pixelShader = FindShaderCacheEntry(reinterpret_cast<XXH64_hash_t>(pipelineState.pixelShader))->guestShader;

                {
                    std::lock_guard lock(g_vertexDeclarationMutex);
                    pipelineState.vertexDeclaration = g_vertexDeclarations[reinterpret_cast<XXH64_hash_t>(pipelineState.vertexDeclaration)];
                }

                if (!g_triangleFanSupported && pipelineState.primitiveTopology == RenderPrimitiveTopology::TRIANGLE_FAN)
                    pipelineState.primitiveTopology = RenderPrimitiveTopology::TRIANGLE_LIST;

                if (Config::GITextureFiltering == EGITextureFiltering::Bicubic)
                    pipelineState.specConstants |= SPEC_CONSTANT_BICUBIC_GI_FILTER;

                // Compile both MSAA and non MSAA variants to work with reflection maps. The render formats are an assumption but it should hold true.
                if (Config::AntiAliasing != EAntiAliasing::None &&
                    pipelineState.renderTargetFormat == RenderFormat::R16G16B16A16_FLOAT && 
                    pipelineState.depthStencilFormat == RenderFormat::D32_FLOAT)
                {
                    auto msaaPipelineState = pipelineState;
                    msaaPipelineState.sampleCount = int32_t(Config::AntiAliasing.Value);

                    if (Config::TransparencyAntiAliasing && (msaaPipelineState.specConstants & SPEC_CONSTANT_ALPHA_TEST) != 0)
                    {
                        msaaPipelineState.enableAlphaToCoverage = true;
                        msaaPipelineState.specConstants &= ~SPEC_CONSTANT_ALPHA_TEST;
                        msaaPipelineState.specConstants |= SPEC_CONSTANT_ALPHA_TO_COVERAGE;
                    }

                    SanitizePipelineState(msaaPipelineState);
                    EnqueueGraphicsPipelineCompilation(msaaPipelineState, emptyHolderPair, "Precompiled Pipeline MSAA");
                }

                // Compile the custom gaussian blur shaders that we pass to the game.
                if (pipelineState.pixelShader != nullptr &&
                    pipelineState.pixelShader->shaderCacheEntry != nullptr &&
                    pipelineState.pixelShader->shaderCacheEntry->hash == 0x4294510C775F4EE8)
                {
                    for (auto& shader : g_gaussianBlurShaders)
                    {
                        pipelineState.pixelShader = shader.get();
                        SanitizePipelineState(pipelineState);
                        EnqueueGraphicsPipelineCompilation(pipelineState, emptyHolderPair, "Precompiled Gaussian Blur Pipeline");
                    }
                }
                else
                {
                    SanitizePipelineState(pipelineState);
                    EnqueueGraphicsPipelineCompilation(pipelineState, emptyHolderPair, "Precompiled Pipeline");
                }
            }

            g_pendingPipelineStateCache = false;
            --g_pendingDataCount;

            if ((--g_compilingDataCount) == 0)
                g_compilingDataCount.notify_all();
        }

        {
            std::lock_guard lock(g_pendingModelMutex);
            localPendingDataQueue.insert(localPendingDataQueue.end(), g_pendingDataQueue.begin(), g_pendingDataQueue.end());
            g_pendingDataQueue.clear();
        }

        bool allHandled = true;

        for (auto& pendingData : localPendingDataQueue)
        {
            if (pendingData.get() != nullptr)
            {
                bool ready = false;

                if (pendingData->m_pVftable.ptr == MODEL_DATA_VFTABLE)
                    ready = CheckMadeAll(*reinterpret_cast<Hedgehog::Mirage::CModelData*>(pendingData.get()));
                else
                    ready = pendingData->IsMadeOne();

                if (ready || pendingData.unique())
                {
                    if (pendingData->m_pVftable.ptr == TERRAIN_MODEL_DATA_VFTABLE)
                    {
                        CompilationArgs args{};
                        args.holderPair.holder.databaseData = pendingData;
                        CompileMeshPipelines(*reinterpret_cast<Hedgehog::Mirage::CTerrainModelData*>(pendingData.get()), args);
                    }
                    else if (pendingData->m_pVftable.ptr == PARTICLE_MATERIAL_VFTABLE)
                    {
                        DatabaseDataHolderPair holderPair;
                        holderPair.holder.databaseData = pendingData;
                        CompileParticleMaterialPipeline(*reinterpret_cast<Hedgehog::Sparkle::CParticleMaterial*>(pendingData.get()), holderPair);
                    }
                    else
                    {
                        assert(pendingData->m_pVftable.ptr == MODEL_DATA_VFTABLE);

                        auto modelData = reinterpret_cast<Hedgehog::Mirage::CModelData*>(pendingData.get());

                        CompilationArgs args{};
                        args.holderPair.holder.databaseData = pendingData;
                        args.noGI = true;
                        args.hasMoreThanOneBone = modelData->m_NodeNum > 1;
                        args.velocityMapQuickStep = strcmp(pendingData->m_TypeAndName.c_str() + 2, "SonicRoot") == 0;

                        // Check for the on screen items, eg. rings going to HUD.
                        auto items = reinterpret_cast<xpointer<const char>*>(g_memory.Translate(0x832A8DD0));
                        for (size_t i = 0; i < 50; i++)
                        {
                            if (strcmp(pendingData->m_TypeAndName.c_str() + 2, (*items).get()) == 0)
                            {
                                args.objectIcon = true;
                                break;
                            }
                            items += 7;
                        }

                        CompileMeshPipelines(*modelData, args);
                    }

                    pendingData = nullptr;
                    --g_pendingDataCount;
                }
                else
                {
                    allHandled = false;
                }
            }
        }

        if (allHandled)
            localPendingDataQueue.clear();
    }
}

static std::thread g_modelConsumerThread(ModelConsumerThread);

#ifdef ASYNC_PSO_DEBUG

PPC_FUNC_IMPL(__imp__sub_82E33330);
PPC_FUNC(sub_82E33330)
{
    auto vertexShaderCode = reinterpret_cast<Hedgehog::Mirage::CVertexShaderCodeData*>(g_memory.Translate(ctx.r4.u32));
    __imp__sub_82E33330(ctx, base);
    reinterpret_cast<GuestShader*>(vertexShaderCode->m_pD3DVertexShader.get())->name = vertexShaderCode->m_TypeAndName.c_str() + 3;
}

PPC_FUNC_IMPL(__imp__sub_82E328D8);
PPC_FUNC(sub_82E328D8)
{
    auto pixelShaderCode = reinterpret_cast<Hedgehog::Mirage::CPixelShaderCodeData*>(g_memory.Translate(ctx.r4.u32));
    __imp__sub_82E328D8(ctx, base);
    reinterpret_cast<GuestShader*>(pixelShaderCode->m_pD3DPixelShader.get())->name = pixelShaderCode->m_TypeAndName.c_str() + 2;
}

#endif

#ifdef PSO_CACHING
class SDLEventListenerForPSOCaching : public SDLEventListener
{
public:
    void OnSDLEvent(SDL_Event* event) override 
    {
        if (event->type != SDL_QUIT)
            return;

        std::lock_guard lock(g_pipelineCacheMutex);
        if (g_pipelineStatesToCache.empty())
            return;

        FILE* f = fopen("send_this_file_to_skyth.txt", "ab");
        if (f != nullptr)
        {
            ankerl::unordered_dense::set<GuestVertexDeclaration*> vertexDeclarations;
            xxHashMap<PipelineState> pipelineStatesToCache;

            for (auto& pipelineState : g_pipelineStatesToCache)
            {
                if (pipelineState.vertexShader->shaderCacheEntry == nullptr ||
                    (pipelineState.pixelShader != nullptr && pipelineState.pixelShader->shaderCacheEntry == nullptr))
                {
                    continue;
                }

                vertexDeclarations.emplace(pipelineState.vertexDeclaration);

                // Mask out the config options.
                pipelineState.sampleCount = 1;
                pipelineState.enableAlphaToCoverage = false;

                pipelineState.specConstants &= ~SPEC_CONSTANT_BICUBIC_GI_FILTER;
                if ((pipelineState.specConstants & SPEC_CONSTANT_ALPHA_TO_COVERAGE) != 0)
                {
                    pipelineState.specConstants &= ~SPEC_CONSTANT_ALPHA_TO_COVERAGE;
                    pipelineState.specConstants |= SPEC_CONSTANT_ALPHA_TEST;
                }

                pipelineStatesToCache.emplace(XXH3_64bits(&pipelineState, sizeof(pipelineState)), pipelineState);
            }

            for (auto vertexDeclaration : vertexDeclarations)
            {
                std::print(f, "static uint8_t g_vertexElements_{:016X}[] = {{", vertexDeclaration->hash);

                auto bytes = reinterpret_cast<uint8_t*>(vertexDeclaration->vertexElements.get());
                for (size_t i = 0; i < vertexDeclaration->vertexElementCount * sizeof(GuestVertexElement); i++)
                    std::print(f, "0x{:X},", bytes[i]);

                std::println(f, "}};");
            }

            for (auto& [pipelineHash, pipelineState] : pipelineStatesToCache)
            {
                std::println(f, "{{ "
                    "reinterpret_cast<GuestShader*>(0x{:X}),"
                    "reinterpret_cast<GuestShader*>(0x{:X}),"
                    "reinterpret_cast<GuestVertexDeclaration*>(0x{:X}),"
                    "{},"
                    "{},"
                    "{},"
                    "RenderBlend::{},"
                    "RenderBlend::{},"
                    "RenderCullMode::{},"
                    "RenderComparisonFunction::{},"
                    "{},"
                    "RenderBlendOperation::{},"
                    "{},"
                    "{},"
                    "RenderBlend::{},"
                    "RenderBlend::{},"
                    "RenderBlendOperation::{},"
                    "0x{:X},"
                    "RenderPrimitiveTopology::{},"
                    "{{ {},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{} }},"
                    "RenderFormat::{},"
                    "RenderFormat::{},"
                    "{},"
                    "{},"
                    "0x{:X} }},",
                    pipelineState.vertexShader->shaderCacheEntry->hash,
                    pipelineState.pixelShader != nullptr ? pipelineState.pixelShader->shaderCacheEntry->hash : 0,
                    pipelineState.vertexDeclaration->hash,
                    pipelineState.instancing,
                    pipelineState.zEnable,
                    pipelineState.zWriteEnable,
                    magic_enum::enum_name(pipelineState.srcBlend),
                    magic_enum::enum_name(pipelineState.destBlend),
                    magic_enum::enum_name(pipelineState.cullMode),
                    magic_enum::enum_name(pipelineState.zFunc),
                    pipelineState.alphaBlendEnable,
                    magic_enum::enum_name(pipelineState.blendOp),
                    pipelineState.slopeScaledDepthBias,
                    pipelineState.depthBias,
                    magic_enum::enum_name(pipelineState.srcBlendAlpha),
                    magic_enum::enum_name(pipelineState.destBlendAlpha),
                    magic_enum::enum_name(pipelineState.blendOpAlpha),
                    pipelineState.colorWriteEnable,
                    magic_enum::enum_name(pipelineState.primitiveTopology),
                    pipelineState.vertexStrides[0],
                    pipelineState.vertexStrides[1],
                    pipelineState.vertexStrides[2],
                    pipelineState.vertexStrides[3],
                    pipelineState.vertexStrides[4],
                    pipelineState.vertexStrides[5],
                    pipelineState.vertexStrides[6],
                    pipelineState.vertexStrides[7],
                    pipelineState.vertexStrides[8],
                    pipelineState.vertexStrides[9],
                    pipelineState.vertexStrides[10],
                    pipelineState.vertexStrides[11],
                    pipelineState.vertexStrides[12],
                    pipelineState.vertexStrides[13],
                    pipelineState.vertexStrides[14],
                    pipelineState.vertexStrides[15],
                    magic_enum::enum_name(pipelineState.renderTargetFormat),
                    magic_enum::enum_name(pipelineState.depthStencilFormat),
                    pipelineState.sampleCount,
                    pipelineState.enableAlphaToCoverage,
                    pipelineState.specConstants);
            }

            fclose(f);
        }
    }
};
SDLEventListenerForPSOCaching g_sdlEventListenerForPSOCaching;
#endif

void VideoConfigValueChangedCallback(IConfigDef* config)
{
    // Config options that require internal resolution resize
    g_needsResize |=
        config == &Config::ResolutionScale ||
        config == &Config::AntiAliasing ||
        config == &Config::ShadowResolution;
}

GUEST_FUNCTION_HOOK(sub_82BD99B0, CreateDevice);

GUEST_FUNCTION_HOOK(sub_82BE6230, DestructResource);

GUEST_FUNCTION_HOOK(sub_82BE9300, LockTextureRect);
GUEST_FUNCTION_HOOK(sub_82BE7780, UnlockTextureRect);

GUEST_FUNCTION_HOOK(sub_82BE6B98, LockVertexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE6BE8, UnlockVertexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE61D0, GetVertexBufferDesc);

GUEST_FUNCTION_HOOK(sub_82BE6CA8, LockIndexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE6CF0, UnlockIndexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE6200, GetIndexBufferDesc);

GUEST_FUNCTION_HOOK(sub_82BE96F0, GetSurfaceDesc);

GUEST_FUNCTION_HOOK(sub_82BE04B0, GetVertexDeclaration);
GUEST_FUNCTION_HOOK(sub_82BE0530, HashVertexDeclaration);

GUEST_FUNCTION_HOOK(sub_82BDA8C0, GuestPresent);
GUEST_FUNCTION_HOOK(sub_82BDD330, GetBackBuffer);

GUEST_FUNCTION_HOOK(sub_82BE9498, CreateTexture);
GUEST_FUNCTION_HOOK(sub_82BE6AD0, CreateVertexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE6BF8, CreateIndexBuffer);
GUEST_FUNCTION_HOOK(sub_82BE95B8, CreateSurface);

GUEST_FUNCTION_HOOK(sub_82BF6400, StretchRect);

GUEST_FUNCTION_HOOK(sub_82BDD9F0, SetRenderTarget);
GUEST_FUNCTION_HOOK(sub_82BDDD38, SetDepthStencilSurface);

GUEST_FUNCTION_HOOK(sub_82BFE4C8, Clear);

GUEST_FUNCTION_HOOK(sub_82BDD8C0, SetViewport);

GUEST_FUNCTION_HOOK(sub_82BE9818, SetTexture);
GUEST_FUNCTION_HOOK(sub_82BDCFB0, SetScissorRect);

GUEST_FUNCTION_HOOK(sub_82BE5900, DrawPrimitive);
GUEST_FUNCTION_HOOK(sub_82BE5CF0, DrawIndexedPrimitive);
GUEST_FUNCTION_HOOK(sub_82BE52F8, DrawPrimitiveUP);

GUEST_FUNCTION_HOOK(sub_82BE0428, CreateVertexDeclaration);
GUEST_FUNCTION_HOOK(sub_82BE02E0, SetVertexDeclaration);

GUEST_FUNCTION_HOOK(sub_82BE1A80, CreateVertexShader);
GUEST_FUNCTION_HOOK(sub_82BE0110, SetVertexShader);

GUEST_FUNCTION_HOOK(sub_82BDD0F8, SetStreamSource);
GUEST_FUNCTION_HOOK(sub_82BDD218, SetIndices);

GUEST_FUNCTION_HOOK(sub_82BE1990, CreatePixelShader);
GUEST_FUNCTION_HOOK(sub_82BDFE58, SetPixelShader);

GUEST_FUNCTION_HOOK(sub_82C003B8, D3DXFillTexture);
GUEST_FUNCTION_HOOK(sub_82C00910, D3DXFillVolumeTexture);

GUEST_FUNCTION_HOOK(sub_82E43FC8, MakePictureData);

GUEST_FUNCTION_HOOK(sub_82E9EE38, SetResolution);

GUEST_FUNCTION_HOOK(sub_82AE2BF8, ScreenShaderInit);

GUEST_FUNCTION_STUB(sub_822C15D8);
GUEST_FUNCTION_STUB(sub_822C1810);
GUEST_FUNCTION_STUB(sub_82BD97A8);
GUEST_FUNCTION_STUB(sub_82BD97E8);
GUEST_FUNCTION_STUB(sub_82BDD370); // SetGammaRamp
GUEST_FUNCTION_STUB(sub_82BE05B8);
GUEST_FUNCTION_STUB(sub_82BE9C98);
GUEST_FUNCTION_STUB(sub_82BEA308);
GUEST_FUNCTION_STUB(sub_82CD5D68);
GUEST_FUNCTION_STUB(sub_82BE9B28);
GUEST_FUNCTION_STUB(sub_82BEA018);
GUEST_FUNCTION_STUB(sub_82BEA7C0);
GUEST_FUNCTION_STUB(sub_82BFFF88); // D3DXFilterTexture
GUEST_FUNCTION_STUB(sub_82BD96D0);
