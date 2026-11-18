#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#ifdef __spirv__

#define g_SrcAlpha_DestAlpha vk::RawBufferLoad<float4>(g_PushConstants.PixelShaderConstants + 2400, 0x10)
#define s0_Texture2DDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 0)
#define s0_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 192)

#else

cbuffer PixelShaderConstants : register(b1, space4)
{
    float4 g_SrcAlpha_DestAlpha : packoffset(c150);
};

cbuffer SharedConstants : register(b2, space4)
{
    uint s0_Texture2DDescriptorIndex : packoffset(c0.x);
    uint s0_SamplerDescriptorIndex : packoffset(c12.x);
    DEFINE_SHARED_CONSTANTS();
};

#endif

float4 shaderMain(
    in float4 iPos : SV_Position,
    in float4 iTexCoord0 : TEXCOORD0) : SV_Target0
{
    Texture2D<float4> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex];
    SamplerState samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex];
    
    float4 color = texture.Sample(samplerState, iTexCoord0.xy);
    
    if (any(or(iTexCoord0.xy < 0.0, iTexCoord0.xy > 1.0)))
        color = float4(0.0, 0.0, 0.0, 1.0);
    
    color.rgb *= color.a * g_SrcAlpha_DestAlpha.x;
    color.a = g_SrcAlpha_DestAlpha.y + (1.0 - color.a) * g_SrcAlpha_DestAlpha.x;
    
    return color;
}
