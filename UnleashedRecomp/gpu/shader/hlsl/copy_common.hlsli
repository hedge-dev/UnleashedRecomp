#pragma once

struct PushConstants
{
    uint ResourceDescriptorIndex;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> g_PushConstants : register(b3, space4);
