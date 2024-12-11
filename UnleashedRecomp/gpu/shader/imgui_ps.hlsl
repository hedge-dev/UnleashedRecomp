#include "imgui_common.hlsli"

float4 DecodeColor(uint color)
{
    return float4(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF) / 255.0;
}

float4 SamplePoint(int2 position)
{
    switch (g_PushConstants.ShaderModifier)
    {
        case IMGUI_SHADER_MODIFIER_SCANLINE:
            {
                if (int(position.y) % 2 == 0)
                    return float4(1.0, 1.0, 1.0, 0.0);
            
                break;
            }
        case IMGUI_SHADER_MODIFIER_CHECKERBOARD:
            {
                int remnantX = int(position.x) % 9;
                int remnantY = int(position.y) % 9;
            
                float4 color = 1.0;
            
                if (remnantX == 0 || remnantY == 0)
                    color.a = 0.0;
            
                if ((remnantY % 2) == 0)
                    color.rgb = 0.5;
            
                return color;
            }
        case IMGUI_SHADER_MODIFIER_SCANLINE_BUTTON:
            {
                if (int(position.y) % 2 == 0)
                    return float4(1.0, 1.0, 1.0, 0.5);
            
                break;
            }
    }
    
    return 1.0;
}

float4 SampleLinear(float2 uvTexspace)
{    
    int2 integerPart = floor(uvTexspace);
    float2 fracPart = frac(uvTexspace);
    
    float4 topLeft = SamplePoint(integerPart + float2(0, 0));
    float4 topRight = SamplePoint(integerPart + float2(1, 0));
    float4 bottomLeft = SamplePoint(integerPart + float2(0, 1));
    float4 bottomRight = SamplePoint(integerPart + float2(1, 1));
    
    float4 top = lerp(topLeft, topRight, fracPart.x);
    float4 bottom = lerp(bottomLeft, bottomRight, fracPart.x);

    return lerp(top, bottom, fracPart.y);
}

float4 PixelAntialiasing(float2 uvTexspace)
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

uint GetTexture2DDescriptorIndex()
{
    return g_PushConstants.Texture2DDescriptorIndex & 0x7FFFFFFF;
}

float ComputeScreenPixelRange(float2 texCoord)
{
    uint width, height;
    g_Texture2DDescriptorHeap[GetTexture2DDescriptorIndex()].GetDimensions(width, height);
    
    float2 unitRange = 4.0 / float2(width, height);
    float2 screenTextureSize = 1.0 / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTextureSize), 1.0);
}

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float4 main(in Interpolators interpolators) : SV_Target
{
    float4 color = interpolators.Color;
    color *= PixelAntialiasing(interpolators.Position.xy - 0.5);
    
    if (g_PushConstants.Texture2DDescriptorIndex != 0)
    {
        float4 texture = g_Texture2DDescriptorHeap[GetTexture2DDescriptorIndex()].Sample(g_SamplerDescriptorHeap[0], interpolators.UV);
        if ((g_PushConstants.Texture2DDescriptorIndex & 0x80000000) != 0)
        {
            float sd = median(texture.r, texture.g, texture.b) - 0.5;
            float screenPixelDistance = ComputeScreenPixelRange(interpolators.UV) * sd;
            color.a *= saturate(screenPixelDistance + 0.5);
            color.a *= texture.a;
        }
        else
        {
            color *= texture;
        }
    }
    
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_MARQUEE_FADE)
    {
        float minAlpha = saturate((interpolators.Position.x - g_PushConstants.BoundsMin.x) / g_PushConstants.Scale.x);
        float maxAlpha = saturate((g_PushConstants.BoundsMax.x - interpolators.Position.x) / g_PushConstants.Scale.x);
        
        color.a *= minAlpha;
        color.a *= maxAlpha;
    }
    else if (any(g_PushConstants.BoundsMin != g_PushConstants.BoundsMax))
    {
        float2 factor = saturate((interpolators.Position.xy - g_PushConstants.BoundsMin) / (g_PushConstants.BoundsMax - g_PushConstants.BoundsMin));
        color *= lerp(DecodeColor(g_PushConstants.GradientTop), DecodeColor(g_PushConstants.GradientBottom), smoothstep(0.0, 1.0, factor.y));
    }
        
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_GRAYSCALE)
        color.rgb = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    
    return color;
}
