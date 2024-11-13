#include "imgui_common.hlsli"
#include "../imgui_common.h"

float4 DecodeColor(uint color)
{
    return float4(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF) / 255.0;
}

float SamplePoint(int2 position)
{
    switch (g_PushConstants.ShaderModifier)
    {
        case IMGUI_SHADER_MODIFIER_SCANLINE:
            {
                if (int(position.y) % 2 == 0)
                    return 0.0;
            
                break;
            }
        case IMGUI_SHADER_MODIFIER_CHECKERBOARD:
            {
                int remnantX = int(position.x) % 9;
                int remnantY = int(position.y) % 9;
            
                if (remnantX == 0 || remnantY == 0)
                    return 0.0;
            
                if ((remnantY % 2) == 0)
                    return 0.5;
            
                break;
            }
        case IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON:
            {
                if (int(position.y) % 2 == 0)
                    return 0.5;
            
                break;
            }
    }
    
    return 1.0;
}

float SampleLinear(float2 uvTexspace)
{    
    int2 integerPart = floor(uvTexspace);
    float2 fracPart = frac(uvTexspace);
    
    float topLeft = SamplePoint(integerPart + float2(0, 0));
    float topRight = SamplePoint(integerPart + float2(1, 0));
    float bottomLeft = SamplePoint(integerPart + float2(0, 1));
    float bottomRight = SamplePoint(integerPart + float2(1, 1));
    
    float top = lerp(topLeft, topRight, fracPart.x);
    float bottom = lerp(bottomLeft, bottomRight, fracPart.x);

    return lerp(top, bottom, fracPart.y);
}

float PixelAntialiasing(float2 uvTexspace)
{
    float2 seam = floor(uvTexspace + 0.5);
    uvTexspace = (uvTexspace - seam) / fwidth(uvTexspace) + seam;
    uvTexspace = clamp(uvTexspace, seam - 0.5, seam + 0.5);
    
    if (g_PushConstants.InverseDisplaySize.x < g_PushConstants.InverseDisplaySize.y)
        uvTexspace *= min(1.0, g_PushConstants.InverseDisplaySize.y * 720.0f);
    else
        uvTexspace *= min(1.0, g_PushConstants.InverseDisplaySize.x * 1280.0f);
    
    return SampleLinear(uvTexspace);
}

float4 main(in Interpolators interpolators) : SV_Target
{
    float4 color = interpolators.Color;
    color.a *= PixelAntialiasing(interpolators.Position.xy - 0.5);
    
    if (g_PushConstants.Texture2DDescriptorIndex != 0)
        color *= g_Texture2DDescriptorHeap[g_PushConstants.Texture2DDescriptorIndex].Sample(g_SamplerDescriptorHeap[0], interpolators.UV);
    
    if (any(g_PushConstants.GradientMin != g_PushConstants.GradientMax))
    {
        float2 factor = saturate((interpolators.Position.xy - g_PushConstants.GradientMin) / (g_PushConstants.GradientMax - g_PushConstants.GradientMin));
        color *= lerp(DecodeColor(g_PushConstants.GradientTop), DecodeColor(g_PushConstants.GradientBottom), factor.y);
    }
        
    return color;
}
