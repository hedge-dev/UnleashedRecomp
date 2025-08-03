#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#define g_SrcAlpha_DestAlpha (*(reinterpret_cast<device float4*>(g_PushConstants.PixelShaderConstants + 2400)))
#define s0_Texture2DDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 0)))
#define s0_SamplerDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 192)))

struct Interpolators
{
    float4 iTexCoord0 [[user(TEXCOORD0)]];
};

[[fragment]]
float4 shaderMain(float4 iPos [[position]],
                  Interpolators input [[stage_in]],
                  constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant SamplerDescriptorHeap* g_SamplerDescriptorHeap [[buffer(3)]],
                  constant PushConstants& g_PushConstants [[buffer(8)]])
{
    texture2d<float> texture = g_Texture2DDescriptorHeap[s0_Texture2DDescriptorIndex].tex;
    sampler samplerState = g_SamplerDescriptorHeap[s0_SamplerDescriptorIndex].samp;
    
    float4 color = texture.sample(samplerState, input.iTexCoord0.xy);
    
    if (any(input.iTexCoord0.xy < 0.0 || input.iTexCoord0.xy > 1.0))
        color = float4(0.0, 0.0, 0.0, 1.0);
    
    color.rgb *= color.a * g_SrcAlpha_DestAlpha.x;
    color.a = g_SrcAlpha_DestAlpha.y + (1.0 - color.a) * g_SrcAlpha_DestAlpha.x;
    
    return color;
}
