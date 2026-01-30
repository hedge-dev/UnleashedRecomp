#include "copy_common.hlsli"

Texture2D<float> g_Texture2DDescriptorHeap[] : register(t0, space0);

float shaderMain(in float4 position : SV_Position) : SV_Depth
{
    return g_Texture2DDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].Load(int3(position.xy, 0));
}
