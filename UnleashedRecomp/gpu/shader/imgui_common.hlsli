#pragma once

struct PushConstants
{
    uint Texture2DDescriptorIndex;
    float2 InverseDisplaySize;
};

Texture2D<float4> g_Texture2DDescriptorHeap[] : register(t0, space0);
SamplerState g_SamplerDescriptorHeap[] : register(s0, space1);
[[vk::push_constant]] ConstantBuffer<PushConstants> g_PushConstants : register(b0, space2);

struct Interpolators
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};
