#pragma once

#include "../../../thirdparty/ShaderRecomp/ShaderRecomp/shader_common.h"

#ifdef __spirv__

#define fZmin vk::RawBufferLoad<float>(g_PushConstants.PixelShaderConstants + 0)
#define fZmax vk::RawBufferLoad<float>(g_PushConstants.PixelShaderConstants + 16)

#define Tex0_ResourceDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 0)
#define Tex1_ResourceDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 4)
#define Tex2_ResourceDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 8)
#define Tex3_ResourceDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 12)
#define Tex4_ResourceDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 16)

#define Tex0_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 64)
#define Tex1_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 68)
#define Tex2_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 72)
#define Tex3_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 76)
#define Tex4_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 80)

#else

cbuffer PixelShaderConstants : register(b1, space4)
{
    float fZmin : packoffset(c0);
    float fZmax : packoffset(c1);
};

cbuffer SharedConstants : register(b2, space4)
{
    uint Tex0_ResourceDescriptorIndex : packoffset(c0.x);
    uint Tex1_ResourceDescriptorIndex : packoffset(c0.y);
    uint Tex2_ResourceDescriptorIndex : packoffset(c0.z);
    uint Tex3_ResourceDescriptorIndex : packoffset(c0.w);
    uint Tex4_ResourceDescriptorIndex : packoffset(c1.x);
    
    uint Tex0_SamplerDescriptorIndex : packoffset(c4.x);
    uint Tex1_SamplerDescriptorIndex : packoffset(c4.y);
    uint Tex2_SamplerDescriptorIndex : packoffset(c4.z);
    uint Tex3_SamplerDescriptorIndex : packoffset(c4.w);
    uint Tex4_SamplerDescriptorIndex : packoffset(c5.x);
    
    DEFINE_SHARED_CONSTANTS();
};

#endif

#define bCsc (g_Booleans & (1 << (16 + 0)))
#define bAmv (g_Booleans & (1 << (16 + 1)))
#define bZmv (g_Booleans & (1 << (16 + 2)))
 
struct VertexShaderInput
{
    [[vk::location(0)]] float4 ObjPos : POSITION;
    [[vk::location(4)]] float2 UV : TEXCOORD;
};

struct Interpolators
{
    float4 ProjPos : SV_Position;
    float2 UV : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 Color : SV_Target0;
    float Depth : SV_Depth;
};
