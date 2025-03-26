#include "copy_common.metali"

struct Texture2DDescriptorHeap
{
    array<texture2d<float>, 1> g [[id(0)]];
};

struct PixelShaderOutput
{
    float oDepth [[depth(any)]];
};

[[fragment]]
PixelShaderOutput shaderMain(float4 position [[position]],
                             constant Texture2DDescriptorHeap& g_Texture2DDescriptorHeap [[buffer(0)]],
                             constant PushConstants& g_PushConstants [[buffer(4)]])
{
    PixelShaderOutput output = PixelShaderOutput{};

    output.oDepth = g_Texture2DDescriptorHeap.g[g_PushConstants.ResourceDescriptorIndex].read(uint2(position.xy), 0).x;

    return output;
}
