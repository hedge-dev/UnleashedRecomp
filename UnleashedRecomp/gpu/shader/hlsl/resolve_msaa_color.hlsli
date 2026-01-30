#pragma once

#include "copy_common.hlsli"

Texture2DMS<float4, SAMPLE_COUNT> g_Texture2DMSDescriptorHeap[] : register(t0, space0);

float4 shaderMain(in float4 position : SV_Position) : SV_Target
{
    float4 result = g_Texture2DMSDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].Load(int2(position.xy), 0);
    
    [unroll] for (int i = 1; i < SAMPLE_COUNT; i++)
        result += g_Texture2DMSDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].Load(int2(position.xy), i);

    return result / SAMPLE_COUNT;
}
