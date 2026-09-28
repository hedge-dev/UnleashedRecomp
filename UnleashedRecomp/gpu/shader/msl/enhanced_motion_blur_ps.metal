#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#define g_BlurRate (*(reinterpret_cast<device float4*>(g_PushConstants.PixelShaderConstants + 2400)))
#define g_ViewportSize (*(reinterpret_cast<device float4*>(g_PushConstants.PixelShaderConstants + 384)))

#define sampColor_Texture2DDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 0)))
#define sampColor_SamplerDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 192)))

#define sampVelocityMap_Texture2DDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 4)))
#define sampVelocityMap_SamplerDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 196)))

#define sampZBuffer_Texture2DDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 8)))
#define sampZBuffer_SamplerDescriptorIndex (*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 200)))

struct Interpolators
{
    float4 texCoord [[user(TEXCOORD0)]];
};

[[fragment]]
float4 shaderMain(float4 position [[position]],
                  Interpolators input [[stage_in]],
                  constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant SamplerDescriptorHeap* g_SamplerDescriptorHeap [[buffer(3)]],
                  constant PushConstants& g_PushConstants [[buffer(8)]])
{
    texture2d<float> sampColor = g_Texture2DDescriptorHeap[sampColor_Texture2DDescriptorIndex].tex;
    texture2d<float> sampVelocityMap = g_Texture2DDescriptorHeap[sampVelocityMap_Texture2DDescriptorIndex].tex;
    texture2d<float> sampZBuffer = g_Texture2DDescriptorHeap[sampZBuffer_Texture2DDescriptorIndex].tex;
    
    sampler sampColor_s = g_SamplerDescriptorHeap[sampColor_SamplerDescriptorIndex].samp;
    sampler sampVelocityMap_s = g_SamplerDescriptorHeap[sampVelocityMap_SamplerDescriptorIndex].samp;
    sampler sampZBuffer_s = g_SamplerDescriptorHeap[sampZBuffer_SamplerDescriptorIndex].samp;
    
    float depth = sampZBuffer.sample(sampZBuffer_s, input.texCoord.xy, level(0)).x;
    float4 velocityMap = sampVelocityMap.sample(sampVelocityMap_s, input.texCoord.xy, level(0));
    float2 velocity = (velocityMap.xz + velocityMap.yw / 255.0) * 2.0 - 1.0;

    int sampleCount = min(64, int(round(length(velocity * g_ViewportSize.xy))));
    float2 sampleOffset = velocity / (float) sampleCount;

    float3 color = sampColor.sample(sampColor_s, input.texCoord.xy, level(0)).rgb;
    int count = 1;

    for (int i = 1; i <= sampleCount; i++)
    {
        float2 sampleCoord = input.texCoord.xy + sampleOffset * i;
        float3 sampleColor = sampColor.sample(sampColor_s, sampleCoord, level(0)).rgb;
        float sampleDepth = sampZBuffer.sample(sampZBuffer_s, sampleCoord, 0).x;

        if (sampleDepth - depth < 0.01)
        {
            color += sampleColor;
            count += 1;
        }
    }

    return float4(color / count, g_BlurRate.x * saturate(dot(abs(velocity), g_ViewportSize.xy) / 8.0) * saturate(float(count - 1)));
}
