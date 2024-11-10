#include "imgui_common.hlsli"

float4 main(in Interpolators interpolators) : SV_Target
{
    float4 color = interpolators.Color;
    
    if (g_PushConstants.Texture2DDescriptorIndex != 0)
        color *= g_Texture2DDescriptorHeap[g_PushConstants.Texture2DDescriptorIndex].Sample(g_SamplerDescriptorHeap[0], interpolators.UV);
    
    return color;
}
