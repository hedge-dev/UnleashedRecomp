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
    if ((g_PushConstants.DisplaySize.x * g_PushConstants.InverseDisplaySize.y) >= (4.0 / 3.0))
        uvTexspace *= g_PushConstants.InverseDisplaySize.y * 720.0;
    else
        uvTexspace *= g_PushConstants.InverseDisplaySize.x * 960.0;

    float2 seam = floor(uvTexspace + 0.5);
    uvTexspace = (uvTexspace - seam) / fwidth(uvTexspace) + seam;
    uvTexspace = clamp(uvTexspace, seam - 0.5, seam + 0.5);
    
    return SampleLinear(uvTexspace - 0.5);
}

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float4 SampleSdfFont(float4 color, Texture2D<float4> texture, float2 uv, float2 screenTexSize)
{
    float4 textureColor = texture.Sample(g_SamplerDescriptorHeap[0], uv);
    
    uint width, height;
    texture.GetDimensions(width, height);
            
    float pxRange = 8.0;
    float2 unitRange = pxRange / float2(width, height);
    float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);
            
    float sd = median(textureColor.r, textureColor.g, textureColor.b) - 0.5;
    float screenPxDistance = screenPxRange * (sd + g_PushConstants.Outline / (pxRange * 1.5));
            
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_TITLE_BEVEL)
    {
        float2 normal = normalize(float3(ddx(sd), ddy(sd), 0.01)).xy;
        float3 rimColor = float3(1, 0.8, 0.29);
        float3 shadowColor = float3(0.84, 0.57, 0);

        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float3 gradient = lerp(color.rgb, cosTheta >= 0.0 ? rimColor : shadowColor, abs(cosTheta));
        color.rgb = lerp(gradient, color.rgb, pow(saturate(sd + 0.77), 32.0));
    }
    else if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_CATEGORY_BEVEL)
    {
        float2 normal = normalize(float3(ddx(sd), ddy(sd), 0.25)).xy;
        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float gradient = 1.0 + cosTheta * 0.5;
        color.rgb = saturate(color.rgb * gradient);
    }
    else if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_TEXT_SKEW)
    {
        float2 normal = normalize(float3(ddx(sd), ddy(sd), 0.5)).xy;
        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float gradient = saturate(1.0 + cosTheta);
        color.rgb = lerp(color.rgb * gradient, color.rgb, pow(saturate(sd + 0.77), 32.0));
    }

    color.a *= saturate(screenPxDistance + 0.5);
    color.a *= textureColor.a;
    
    return color;
}

float4 shaderMain(in Interpolators interpolators) : SV_Target
{
    float4 color = interpolators.Color;
    color *= PixelAntialiasing(interpolators.Position.xy - g_PushConstants.ProceduralOrigin);
    
    if (g_PushConstants.Texture2DDescriptorIndex != 0)
    {
        Texture2D<float4> texture = g_Texture2DDescriptorHeap[g_PushConstants.Texture2DDescriptorIndex & 0x7FFFFFFF];
        
        if ((g_PushConstants.Texture2DDescriptorIndex & 0x80000000) != 0)
        {
            if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_LOW_QUALITY_TEXT)
            {
                float scale;
                float invScale;
                
                if ((g_PushConstants.DisplaySize.x * g_PushConstants.InverseDisplaySize.y) >= (4.0 / 3.0))
                {
                    scale = g_PushConstants.InverseDisplaySize.y * 720.0;
                    invScale = g_PushConstants.DisplaySize.y / 720.0;
                }
                else
                {
                    scale = g_PushConstants.InverseDisplaySize.x * 960.0;
                    invScale = g_PushConstants.DisplaySize.x / 960.0;
                }
                
                float2 lowQualityPosition = (interpolators.Position.xy - 0.5) * scale;                
                float2 fracPart = frac(lowQualityPosition);
                
                float2 uvStep = fwidth(interpolators.UV) * invScale;
                float2 lowQualityUV = interpolators.UV - fracPart * uvStep;
                float2 screenTexSize = 1.0 / uvStep;
                
                float4 topLeft = SampleSdfFont(color, texture, lowQualityUV + float2(0, 0), screenTexSize);
                float4 topRight = SampleSdfFont(color, texture, lowQualityUV + float2(uvStep.x, 0), screenTexSize);
                float4 bottomLeft = SampleSdfFont(color, texture, lowQualityUV + float2(0, uvStep.y), screenTexSize);
                float4 bottomRight = SampleSdfFont(color, texture, lowQualityUV + uvStep.xy, screenTexSize);
                
                float4 top = lerp(topLeft, topRight, fracPart.x);
                float4 bottom = lerp(bottomLeft, bottomRight, fracPart.x);
                
                color = lerp(top, bottom, fracPart.y);
            }
            else
            {
                color = SampleSdfFont(color, texture, interpolators.UV, 1.0 / fwidth(interpolators.UV));
            }
        }
        else
        {
            color *= texture.Sample(g_SamplerDescriptorHeap[0], interpolators.UV);
        }
    }
    
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_HORIZONTAL_MARQUEE_FADE)
    {
        float minAlpha = saturate((interpolators.Position.x - g_PushConstants.BoundsMin.x) / g_PushConstants.Scale.x);
        float maxAlpha = saturate((g_PushConstants.BoundsMax.x - interpolators.Position.x) / g_PushConstants.Scale.y);
        
        color.a *= minAlpha;
        color.a *= maxAlpha;
    }
    else if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_VERTICAL_MARQUEE_FADE)
    {
        float minAlpha = saturate((interpolators.Position.y - g_PushConstants.BoundsMin.y) / g_PushConstants.Scale.x);
        float maxAlpha = saturate((g_PushConstants.BoundsMax.y - interpolators.Position.y) / g_PushConstants.Scale.y);
        
        color.a *= minAlpha;
        color.a *= maxAlpha;
    }
    else if (any(g_PushConstants.BoundsMin != g_PushConstants.BoundsMax))
    {
        float2 factor = saturate((interpolators.Position.xy - g_PushConstants.BoundsMin) / (g_PushConstants.BoundsMax - g_PushConstants.BoundsMin));
        
        if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_RECTANGLE_BEVEL)
        {
            float bevelSize = 0.9;

            float shadow = saturate((factor.x - bevelSize) / (1.0 - bevelSize));
            shadow = max(shadow, saturate((factor.y - bevelSize) / (1.0 - bevelSize)));

            float rim = saturate((1.0 - factor.x - bevelSize) / (1.0 - bevelSize));
            rim = max(rim, saturate((1.0 - factor.y - bevelSize) / (1.0 - bevelSize)));

            float3 rimColor = float3(1, 0.8, 0.29);
            float3 shadowColor = float3(0.84, 0.57, 0);

            color.rgb = lerp(color.rgb, rimColor, smoothstep(0.0, 1.0, rim));
            color.rgb = lerp(color.rgb, shadowColor, smoothstep(0.0, 1.0, shadow));
        }
        else
        {
            float4 top = lerp(DecodeColor(g_PushConstants.GradientTopLeft), DecodeColor(g_PushConstants.GradientTopRight), smoothstep(0.0, 1.0, factor.x));
            float4 bottom = lerp(DecodeColor(g_PushConstants.GradientBottomLeft), DecodeColor(g_PushConstants.GradientBottomRight), smoothstep(0.0, 1.0, factor.x));
            color *= lerp(top, bottom, smoothstep(0.0, 1.0, factor.y));
        }
    }
        
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_GRAYSCALE)
        color.rgb = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    
    return color;
}
