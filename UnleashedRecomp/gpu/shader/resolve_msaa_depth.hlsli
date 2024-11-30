#pragma once

struct PushConstants
{
    uint ResourceDescriptorIndex;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> g_PushConstants : register(b3, space4);

Texture2DMS<float, SAMPLE_COUNT> g_Texture2DMSDescriptorHeap[] : register(t0, space0);

float main(in float4 position : SV_Position) : SV_Depth
{
    float result = g_Texture2DMSDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].Load(int2(position.xy), 0);
    
    [unroll] for (int i = 1; i < SAMPLE_COUNT; i++)
        result = min(result, g_Texture2DMSDescriptorHeap[g_PushConstants.ResourceDescriptorIndex].Load(int2(position.xy), i));

    return result;
}
