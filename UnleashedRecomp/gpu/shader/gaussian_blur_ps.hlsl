#include "../../../thirdparty/ShaderRecomp/ShaderRecomp/shader_common.h"

#ifdef __spirv__

#define g_ViewportSize vk::RawBufferLoad<float4>(g_PushConstants.PixelShaderConstants + 384, 0x10)
#define g_offsets(INDEX) select((INDEX) < 74, vk::RawBufferLoad<float4>(g_PushConstants.PixelShaderConstants + (150 + min(INDEX, 73)) * 16, 0x10), 0.0)
#define g_weights vk::RawBufferLoad<float4>(g_PushConstants.PixelShaderConstants + 2656, 0x10)

#define s0_Texture2DDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 0)
#define s0_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 192)

#else

cbuffer PixelShaderConstants : register(b1, space4)
{
    float4 g_ViewportSize : packoffset(c24);
    float4 g_offsets[2] : packoffset(c150);
#define g_offsets(INDEX) select((INDEX) < 74, g_offsets[min(INDEX, 73)], 0.0)
    float4 g_weights : packoffset(c166);
};

cbuffer SharedConstants : register(b2, space4)
{
    uint s0_Texture2DDescriptorIndex : packoffset(c0.x);
    uint s0_SamplerDescriptorIndex : packoffset(c12.x);
	DEFINE_SHARED_CONSTANTS();
};

#endif

float4 main(in float4 iPosition : SV_Position, in float4 iTexCoord0 : TEXCOORD0) : SV_Target
{
    Texture2D<float4> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex];
    SamplerState samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex];
    
    float scaleFactor = g_ViewportSize.y / 360.0;
    
    float2 offsets[5];
    offsets[0] = g_offsets(0).xy * scaleFactor;
    offsets[2] = g_offsets(0).zw * scaleFactor;
    offsets[4] = g_offsets(1).xy * scaleFactor;
    
    offsets[1] = lerp(offsets[2], offsets[0], 0.5);
    offsets[3] = lerp(offsets[2], offsets[4], 0.5);
    
    float weights[5];
    weights[0] = 0.1131226076;
    weights[1] = 0.2360540033;
    weights[2] = 0.3016467782;
    weights[3] = 0.2360540033;
    weights[4] = 0.1131226076;

    float4 c0 = texture.Sample(samplerState, iTexCoord0.xy + offsets[0]) * weights[0];
    float4 c1 = texture.Sample(samplerState, iTexCoord0.xy + offsets[1]) * weights[1];
    float4 c2 = texture.Sample(samplerState, iTexCoord0.xy + offsets[2]) * weights[2];
    float4 c3 = texture.Sample(samplerState, iTexCoord0.xy + offsets[3]) * weights[3];
    float4 c4 = texture.Sample(samplerState, iTexCoord0.xy + offsets[4]) * weights[4];
    
    return c0 + c1 + c2 + c3 + c4;
}
