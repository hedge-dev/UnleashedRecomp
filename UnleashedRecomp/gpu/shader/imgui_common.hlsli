#pragma once

#include "../imgui/imgui_common.h"

struct PushConstants
{
    float2 BoundsMin;
    float2 BoundsMax;
    uint GradientTop;
    uint GradientBottom;
    uint ShaderModifier;
    uint Texture2DDescriptorIndex;
    float2 InverseDisplaySize;
    float2 Origin;
    float2 Scale;
    float Outline;
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
