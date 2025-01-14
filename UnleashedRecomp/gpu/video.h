#pragma once

//#define ASYNC_PSO_DEBUG
#define PSO_CACHING
//#define PSO_CACHING_CLEANUP

#include "rhi/plume_render_interface.h"

#define D3DCLEAR_TARGET  0x1
#define D3DCLEAR_ZBUFFER 0x10

#define LOAD_ZSTD_TEXTURE(name) LoadTexture(decompressZstd(name, name##_uncompressed_size).get(), name##_uncompressed_size)

using namespace plume;

struct Video
{
    static void CreateHostDevice(const char *sdlVideoDriver);
    static void WaitOnSwapChain();
    static void Present();
    static void StartPipelinePrecompilation();
    static void WaitForGPU();
};

struct GuestSamplerState
{
    be<uint32_t> data[6];
};

struct GuestDevice
{
    be<uint64_t> dirtyFlags[8];

    be<uint32_t> setRenderStateFunctions[0x65];
    uint32_t setSamplerStateFunctions[0x14];

    uint8_t padding224[0x25C];

    GuestSamplerState samplerStates[0x20];

    uint32_t vertexShaderFloatConstants[0x400];
    uint32_t pixelShaderFloatConstants[0x400];

    be<uint32_t> vertexShaderBoolConstants[0x4];
    be<uint32_t> pixelShaderBoolConstants[0x4];

    uint8_t padding27A0[0x68C];
    be<uint32_t> vertexDeclaration;
    uint8_t padding2E30[0x338];
    struct
    {
        be<float> x;
        be<float> y;
        be<float> width;
        be<float> height;
        be<float> minZ;
        be<float> maxZ;
    } viewport;
    uint8_t padding3180[0x2C80];
};

static_assert(sizeof(GuestDevice) == 0x5E00);

enum class ResourceType
{
    Texture,
    VolumeTexture,
    VertexBuffer,
    IndexBuffer,
    RenderTarget,
    DepthStencil,
    VertexDeclaration,
    VertexShader,
    PixelShader
};

struct GuestResource
{
    uint32_t unused = 0;
    be<uint32_t> refCount = 1;
    ResourceType type;

    GuestResource(ResourceType type) : type(type) 
    {
    }

    void AddRef()
    {
        std::atomic_ref atomicRef(refCount.value);

        uint32_t originalValue, incrementedValue;
        do
        {
            originalValue = refCount.value;
            incrementedValue = ByteSwap(ByteSwap(originalValue) + 1);
        } while (!atomicRef.compare_exchange_weak(originalValue, incrementedValue));
    }

    void Release()
    {
        std::atomic_ref atomicRef(refCount.value);

        uint32_t originalValue, decrementedValue;
        do
        {
            originalValue = refCount.value;
            decrementedValue = ByteSwap(ByteSwap(originalValue) - 1);
        } while (!atomicRef.compare_exchange_weak(originalValue, decrementedValue));

        // Normally we are supposed to release here, so only use this
        // function when you know you won't be the one destructing it.
    }
};

enum GuestFormat
{
    D3DFMT_A16B16G16R16F = 0x1A22AB60,
    D3DFMT_A16B16G16R16F_2 = 0x1A2201BF,
    D3DFMT_A8B8G8R8 = 0x1A200186,
    D3DFMT_A8R8G8B8 = 0x18280186,
    D3DFMT_D24FS8 = 0x1A220197,
    D3DFMT_D24S8 = 0x2D200196,
    D3DFMT_G16R16F = 0x2D22AB9F,
    D3DFMT_G16R16F_2 = 0x2D20AB8D,
    D3DFMT_INDEX16 = 1,
    D3DFMT_INDEX32 = 6,
    D3DFMT_L8 = 0x28000102,
    D3DFMT_L8_2 = 0x28000002,
    D3DFMT_X8R8G8B8 = 0x28280086,
    D3DFMT_UNKNOWN = 0xFFFFFFFF
};

struct GuestBaseTexture : GuestResource
{
    std::unique_ptr<RenderTexture> textureHolder;
    RenderTexture* texture = nullptr;
    std::unique_ptr<RenderTextureView> textureView;
    uint32_t width = 0;
    uint32_t height = 0;
    RenderFormat format = RenderFormat::UNKNOWN;
    uint32_t descriptorIndex = 0;
    RenderTextureLayout layout = RenderTextureLayout::UNKNOWN;

    GuestBaseTexture(ResourceType type) : GuestResource(type)
    {
    }
};

// Texture/VolumeTexture
struct GuestTexture : GuestBaseTexture
{
    uint32_t depth = 0;
    RenderTextureViewDimension viewDimension = RenderTextureViewDimension::UNKNOWN;
    void* mappedMemory = nullptr;
    std::unique_ptr<RenderFramebuffer> framebuffer;
    std::unique_ptr<GuestTexture> patchedTexture;
};

struct GuestLockedRect
{
    be<uint32_t> pitch;
    be<uint32_t> bits;
};

struct GuestBufferDesc
{
    be<uint32_t> format;
    be<uint32_t> type;
    be<uint32_t> usage;
    be<uint32_t> pool;
    be<uint32_t> size;
    be<uint32_t> fvf;
};

// VertexBuffer/IndexBuffer
struct GuestBuffer : GuestResource
{
    std::unique_ptr<RenderBuffer> buffer;
    void* mappedMemory = nullptr;
    uint32_t dataSize = 0;
    RenderFormat format = RenderFormat::UNKNOWN;
    uint32_t guestFormat = 0;
    bool lockedReadOnly = false;
};

struct GuestSurfaceDesc
{
    be<uint32_t> format;
    be<uint32_t> type;
    be<uint32_t> usage;
    be<uint32_t> pool;
    be<uint32_t> multiSampleType;
    be<uint32_t> multiSampleQuality;
    be<uint32_t> width;
    be<uint32_t> height;
};

// RenderTarget/DepthStencil
struct GuestSurface : GuestBaseTexture
{
    uint32_t guestFormat = 0;
    ankerl::unordered_dense::map<const RenderTexture*, std::unique_ptr<RenderFramebuffer>> framebuffers;
    RenderSampleCounts sampleCount = RenderSampleCount::COUNT_1;
};

enum GuestDeclType
{
    D3DDECLTYPE_FLOAT1 = 0x2C83A4,
    D3DDECLTYPE_FLOAT2 = 0x2C23A5,
    D3DDECLTYPE_FLOAT3 = 0x2A23B9,
    D3DDECLTYPE_FLOAT4 = 0x1A23A6,
    D3DDECLTYPE_D3DCOLOR = 0x182886,
    D3DDECLTYPE_UBYTE4 = 0x1A2286,
    D3DDECLTYPE_UBYTE4_2 = 0x1A2386,
    D3DDECLTYPE_SHORT2 = 0x2C2359,
    D3DDECLTYPE_SHORT4 = 0x1A235A,
    D3DDECLTYPE_UBYTE4N = 0x1A2086,
    D3DDECLTYPE_UBYTE4N_2 = 0x1A2186,
    D3DDECLTYPE_SHORT2N = 0x2C2159,
    D3DDECLTYPE_SHORT4N = 0x1A215A,
    D3DDECLTYPE_USHORT2N = 0x2C2059,
    D3DDECLTYPE_USHORT4N = 0x1A205A,
    D3DDECLTYPE_UINT1 = 0x2C82A1,
    D3DDECLTYPE_UDEC3 = 0x2A2287,
    D3DDECLTYPE_DEC3N = 0x2A2187,
    D3DDECLTYPE_DEC3N_2 = 0x2A2190,
    D3DDECLTYPE_DEC3N_3 = 0x2A2390,
    D3DDECLTYPE_FLOAT16_2 = 0x2C235F,
    D3DDECLTYPE_FLOAT16_4 = 0x1A2360,
    D3DDECLTYPE_UNUSED = 0xFFFFFFFF
};

enum GuestDeclUsage
{
    D3DDECLUSAGE_POSITION = 0,
    D3DDECLUSAGE_BLENDWEIGHT = 1,
    D3DDECLUSAGE_BLENDINDICES = 2,
    D3DDECLUSAGE_NORMAL = 3,
    D3DDECLUSAGE_PSIZE = 4,
    D3DDECLUSAGE_TEXCOORD = 5,
    D3DDECLUSAGE_TANGENT = 6,
    D3DDECLUSAGE_BINORMAL = 7,
    D3DDECLUSAGE_TESSFACTOR = 8,
    D3DDECLUSAGE_POSITIONT = 9,
    D3DDECLUSAGE_COLOR = 10,
    D3DDECLUSAGE_FOG = 11,
    D3DDECLUSAGE_DEPTH = 12,
    D3DDECLUSAGE_SAMPLE = 13
};

struct GuestVertexElement
{
    be<uint16_t> stream;
    be<uint16_t> offset;
    be<uint32_t> type;
    uint8_t method;
    uint8_t usage;
    uint8_t usageIndex;
    uint8_t padding;
};

#define D3DDECL_END() { 255, 0, 0xFFFFFFFF, 0, 0, 0 }

struct GuestVertexDeclaration : GuestResource
{
    XXH64_hash_t hash = 0;
    std::unique_ptr<RenderInputElement[]> inputElements;
    std::unique_ptr<GuestVertexElement[]> vertexElements;
    uint32_t inputElementCount = 0;
    uint32_t vertexElementCount = 0;
    uint32_t swappedTexcoords = 0;
    bool hasR11G11B10Normal = false;
    bool vertexStreams[16]{};
    uint32_t indexVertexStream = 0;
};

// VertexShader/PixelShader
struct GuestShader : GuestResource
{
    Mutex mutex;
    std::unique_ptr<RenderShader> shader;
    struct ShaderCacheEntry* shaderCacheEntry = nullptr;
    ankerl::unordered_dense::map<uint32_t, std::unique_ptr<RenderShader>> linkedShaders;
#ifdef SWA_D3D12
    std::vector<ComPtr<IDxcBlob>> shaderBlobs;
    ComPtr<IDxcBlobEncoding> libraryBlob;
#endif
#ifdef ASYNC_PSO_DEBUG
    const char* name = "<unknown>";
#endif
};

struct GuestViewport
{
    be<uint32_t> x;
    be<uint32_t> y;
    be<uint32_t> width;
    be<uint32_t> height;
    be<float> minZ;
    be<float> maxZ;
};

struct GuestRect
{
    be<int32_t> left;
    be<int32_t> top;
    be<int32_t> right;
    be<int32_t> bottom;
};

enum GuestRenderState
{
    D3DRS_ZENABLE = 40,
    D3DRS_ZFUNC = 44,
    D3DRS_ZWRITEENABLE = 48,
    D3DRS_CULLMODE = 56,
    D3DRS_ALPHABLENDENABLE = 60,
    D3DRS_SRCBLEND = 72,
    D3DRS_DESTBLEND = 76,
    D3DRS_BLENDOP = 80,
    D3DRS_SRCBLENDALPHA = 84,
    D3DRS_DESTBLENDALPHA = 88,
    D3DRS_BLENDOPALPHA = 92,
    D3DRS_ALPHATESTENABLE = 96,
    D3DRS_ALPHAREF = 100,
    D3DRS_SCISSORTESTENABLE = 200,
    D3DRS_SLOPESCALEDEPTHBIAS = 204,
    D3DRS_DEPTHBIAS = 208,
    D3DRS_COLORWRITEENABLE = 212
};

enum GuestCullMode
{
    D3DCULL_NONE = 0,
    D3DCULL_CW = 2,
    D3DCULL_NONE_2 = 4,
    D3DCULL_CCW = 6
};

enum GuestBlendMode
{
    D3DBLEND_ZERO = 0,
    D3DBLEND_ONE = 1,
    D3DBLEND_SRCCOLOR = 4,
    D3DBLEND_INVSRCCOLOR = 5,
    D3DBLEND_SRCALPHA = 6,
    D3DBLEND_INVSRCALPHA = 7,
    D3DBLEND_DESTCOLOR = 8,
    D3DBLEND_INVDESTCOLOR = 9,
    D3DBLEND_DESTALPHA = 10,
    D3DBLEND_INVDESTALPHA = 11
};

enum GuestBlendOp
{
    D3DBLENDOP_ADD = 0,
    D3DBLENDOP_SUBTRACT = 1,
    D3DBLENDOP_MIN = 2,
    D3DBLENDOP_MAX = 3,
    D3DBLENDOP_REVSUBTRACT = 4
};

enum GuestCmpFunc
{
    D3DCMP_NEVER = 0,
    D3DCMP_LESS = 1,
    D3DCMP_EQUAL = 2,
    D3DCMP_LESSEQUAL = 3,
    D3DCMP_GREATER = 4,
    D3DCMP_NOTEQUAL = 5,
    D3DCMP_GREATEREQUAL = 6,
    D3DCMP_ALWAYS = 7
};

enum GuestPrimitiveType
{
    D3DPT_POINTLIST = 1,
    D3DPT_LINELIST = 2,
    D3DPT_LINESTRIP = 3,
    D3DPT_TRIANGLELIST = 4,
    D3DPT_TRIANGLEFAN = 5,
    D3DPT_TRIANGLESTRIP = 6,
    D3DPT_QUADLIST = 13
};

enum GuestTextureFilterType
{
    D3DTEXF_POINT = 0,
    D3DTEXF_LINEAR = 1,
    D3DTEXF_NONE = 2
};

enum GuestTextureAddress
{
    D3DTADDRESS_WRAP = 0,
    D3DTADDRESS_MIRROR = 1,
    D3DTADDRESS_CLAMP = 2,
    D3DTADDRESS_MIRRORONCE = 3,
    D3DTADDRESS_BORDER = 6
};

inline bool g_needsResize;

extern std::unique_ptr<GuestTexture> LoadTexture(const uint8_t* data, size_t dataSize, RenderComponentMapping componentMapping = RenderComponentMapping());

extern void VideoConfigValueChangedCallback(class IConfigDef* config);
