#include "copy_common.metali"

struct Texture2DDescriptorHeap
{
    texture2d<float> tex;
};

struct PixelShaderOutput
{
    float oDepth [[depth(any)]];
};

[[fragment]]
PixelShaderOutput shaderMain(float4 position [[position]],
                             constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                             constant PushConstants& g_PushConstants [[buffer(8)]])
{
    PixelShaderOutput output = PixelShaderOutput{};

    output.oDepth = g_Texture2DDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].tex.read(uint2(position.xy), 0).x;

    return output;
}
