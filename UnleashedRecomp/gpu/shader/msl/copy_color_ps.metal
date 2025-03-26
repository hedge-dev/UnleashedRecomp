#include "copy_common.metali"

struct Texture2DDescriptorHeap
{
    array<texture2d<float>, 1> g [[id(0)]];
};

[[fragment]]
float4 shaderMain(float4 position [[position]],
                  constant Texture2DDescriptorHeap& g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant PushConstants& g_PushConstants [[buffer(4)]])
{
    return g_Texture2DDescriptorHeap.g[g_PushConstants.ResourceDescriptorIndex].read(uint2(position.xy), 0);
}
