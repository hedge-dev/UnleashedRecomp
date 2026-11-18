#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#define s0_Texture2DDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 0)))
#define s0_SamplerDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 192)))

struct Interpolators
{
    float4 iPosition [[position]];
    float4 iTexCoord0 [[user(TEXCOORD0)]];
    float4 iTexCoord1 [[user(TEXCOORD1)]];
};

[[fragment]]
float4 shaderMain(Interpolators input [[stage_in]],
                  constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant SamplerDescriptorHeap* g_SamplerDescriptorHeap [[buffer(3)]],
                  constant PushConstants& g_PushConstants [[buffer(8)]])
{
    texture2d<float> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex].tex;
    sampler samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex].samp;

    uint2 dimensions = getTexture2DDimensions(texture);

    // https://www.shadertoy.com/view/csX3RH
    float2 uvTexspace = input.iTexCoord1.xy * float2(dimensions);
    float2 seam = floor(uvTexspace + 0.5);
    uvTexspace = (uvTexspace - seam) / fwidth(uvTexspace) + seam;
    uvTexspace = clamp(uvTexspace, seam - 0.5, seam + 0.5);
    float2 texCoord = uvTexspace / float2(dimensions);
    
    float4 color = texture.sample(samplerState, texCoord);
    color *= input.iTexCoord0;
    
    // The game enables alpha test for CSD, but the alpha threshold doesn't seem to be assigned anywhere? Weird.
    clip(color.a - g_AlphaThreshold);
    
    return color;
}
