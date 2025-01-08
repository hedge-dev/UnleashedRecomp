#include "../../../tools/ShaderRecomp/ShaderRecomp/shader_common.h"

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

#ifdef __INTELLISENSE__
#define KERNEL_SIZE 5
#endif

#define PI 3.14159265358979323846

float ComputeWeight(float x)
{
    float std = 0.952;
    return exp(-(x * x) / (2.0 * std * std)) / (std * sqrt(2.0 * PI));
}

float4 main(in float4 iPosition : SV_Position, in float4 iTexCoord0 : TEXCOORD0) : SV_Target
{
    Texture2D<float4> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex];
    SamplerState samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex];
    
    float scale;
    if ((g_ViewportSize.x * g_ViewportSize.w) >= (16.0 / 9.0))
        scale = g_ViewportSize.y / 360.0;
    else
        scale = g_ViewportSize.x / 640.0;
    
    float2 offsets[3];
    offsets[0] = g_offsets(0).xy * scale;
    offsets[1] = g_offsets(0).zw * scale;
    offsets[2] = g_offsets(1).xy * scale;
    
    float4 color = 0.0;
    float weightSum = 0.0;
    
    [unroll]
    for (int i = 0; i < KERNEL_SIZE; i++)
    {
        float step = i / float(KERNEL_SIZE - 1);
        float scaled = step * 2;
        float2 offset = lerp(offsets[int(scaled)], offsets[min(int(scaled) + 1, 2)], frac(scaled));
        float offsetScale = 1.0 / 0.75;
        float weight = ComputeWeight(lerp(-offsetScale, offsetScale, step));
        color += texture.SampleLevel(samplerState, iTexCoord0.xy + offset, 0) * weight;
        weightSum += weight;
    }
    
    return color / weightSum;
}
