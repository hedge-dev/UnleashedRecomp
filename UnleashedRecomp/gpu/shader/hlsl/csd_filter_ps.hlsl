#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#ifdef __spirv__

#define s0_Texture2DDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 0)
#define s0_SamplerDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 192)

#else

cbuffer SharedConstants : register(b2, space4)
{
    uint s0_Texture2DDescriptorIndex : packoffset(c0.x);
    uint s0_SamplerDescriptorIndex : packoffset(c12.x);
	DEFINE_SHARED_CONSTANTS();
};

#endif

float4 shaderMain(
	in float4 iPosition : SV_Position,
	in float4 iTexCoord0 : TEXCOORD0,
	in float4 iTexCoord1 : TEXCOORD1) : SV_Target
{
    Texture2D<float4> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex];
    SamplerState samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex];
    
    uint2 dimensions;
    texture.GetDimensions(dimensions.x, dimensions.y);
    
    // https://www.shadertoy.com/view/csX3RH
    float2 uvTexspace = iTexCoord1.xy * dimensions;
    float2 seam = floor(uvTexspace + 0.5);
    uvTexspace = (uvTexspace - seam) / fwidth(uvTexspace) + seam;
    uvTexspace = clamp(uvTexspace, seam - 0.5, seam + 0.5);
    float2 texCoord = uvTexspace / dimensions;
    
    float4 color = texture.Sample(samplerState, texCoord);
    color *= iTexCoord0;
    
    // The game enables alpha test for CSD, but the alpha threshold doesn't seem to be assigned anywhere? Weird.
    clip(color.a - g_AlphaThreshold);
    
    return color;
}
