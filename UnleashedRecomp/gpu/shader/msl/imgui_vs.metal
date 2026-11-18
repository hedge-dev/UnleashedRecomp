#include "imgui_common.metali"

struct VertexStageIn
{
    float2 position [[attribute(0)]];
    float2 uv [[attribute(1)]];
    float4 color [[attribute(2)]];
};

[[vertex]]
Interpolators shaderMain(VertexStageIn input [[stage_in]],
                         constant PushConstants& g_PushConstants [[buffer(8)]])
{
    Interpolators interpolators = Interpolators{};

    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_TEXT_SKEW)
    {
        if (input.position.y < g_PushConstants.Origin.y)
            input.position.x += g_PushConstants.Scale.x;
    }
    else if (g_PushConstants.ShaderModifier != IMGUI_SHADER_MODIFIER_HORIZONTAL_MARQUEE_FADE && 
        g_PushConstants.ShaderModifier != IMGUI_SHADER_MODIFIER_VERTICAL_MARQUEE_FADE)
    {
        input.position.xy = g_PushConstants.Origin + (input.position.xy - g_PushConstants.Origin) * g_PushConstants.Scale;
    }
    
    interpolators.Position = float4(input.position.xy * g_PushConstants.InverseDisplaySize * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    interpolators.UV = input.uv;
    interpolators.Color = input.color;

    return interpolators;
}
