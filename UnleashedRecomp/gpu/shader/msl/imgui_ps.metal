#include "imgui_common.metali"

float4 DecodeColor(uint color)
{
    return float4(color & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, (color >> 24) & 0xFF) / 255.0;
}

float4 SamplePoint(int2 position, constant PushConstants& g_PushConstants)
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

float4 SampleLinear(float2 uvTexspace, constant PushConstants& g_PushConstants)
{    
    int2 integerPart = int2(floor(uvTexspace));
    float2 fracPart = fract(uvTexspace);
    
    float4 topLeft = SamplePoint(integerPart + int2(0, 0), g_PushConstants);
    float4 topRight = SamplePoint(integerPart + int2(1, 0), g_PushConstants);
    float4 bottomLeft = SamplePoint(integerPart + int2(0, 1), g_PushConstants);
    float4 bottomRight = SamplePoint(integerPart + int2(1, 1), g_PushConstants);
    
    float4 top = mix(topLeft, topRight, fracPart.x);
    float4 bottom = mix(bottomLeft, bottomRight, fracPart.x);

    return mix(top, bottom, fracPart.y);
}

float4 PixelAntialiasing(float2 uvTexspace, constant PushConstants& g_PushConstants)
{    
    if ((g_PushConstants.DisplaySize.x * g_PushConstants.InverseDisplaySize.y) >= (4.0 / 3.0))
        uvTexspace *= g_PushConstants.InverseDisplaySize.y * 720.0;
    else
        uvTexspace *= g_PushConstants.InverseDisplaySize.x * 960.0;

    float2 seam = floor(uvTexspace + 0.5);
    uvTexspace = (uvTexspace - seam) / fwidth(uvTexspace) + seam;
    uvTexspace = clamp(uvTexspace, seam - 0.5, seam + 0.5);
    
    return SampleLinear(uvTexspace - 0.5, g_PushConstants);
}

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float4 SampleSdfFont(float4 color, texture2d<float> texture, float2 uv, float2 screenTexSize,
                     constant SamplerDescriptorHeap* g_SamplerDescriptorHeap,
                     constant PushConstants& g_PushConstants)
{
    float4 textureColor = texture.sample(g_SamplerDescriptorHeap[0].samp, uv);
    
    uint width = texture.get_width();
    uint height = texture.get_height();
            
    float pxRange = 8.0;
    float2 unitRange = pxRange / float2(width, height);
    float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);
            
    float sd = median(textureColor.r, textureColor.g, textureColor.b) - 0.5;
    float screenPxDistance = screenPxRange * (sd + g_PushConstants.Outline / (pxRange * 1.5));
            
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_TITLE_BEVEL)
    {
        float2 normal = normalize(float3(dfdx(sd), dfdy(sd), 0.01)).xy;
        float3 rimColor = float3(1, 0.8, 0.29);
        float3 shadowColor = float3(0.84, 0.57, 0);

        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float3 gradient = mix(color.rgb, cosTheta >= 0.0 ? rimColor : shadowColor, abs(cosTheta));
        color.rgb = mix(gradient, color.rgb, pow(saturate(sd + 0.77), 32.0));
    }
    else if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_CATEGORY_BEVEL)
    {
        float2 normal = normalize(float3(dfdx(sd), dfdy(sd), 0.25)).xy;
        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float gradient = 1.0 + cosTheta * 0.5;
        color.rgb = saturate(color.rgb * gradient);
    }
    else if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_TEXT_SKEW)
    {
        float2 normal = normalize(float3(dfdx(sd), dfdy(sd), 0.5)).xy;
        float cosTheta = dot(normal, normalize(float2(1, 1)));
        float gradient = saturate(1.0 + cosTheta);
        color.rgb = mix(color.rgb * gradient, color.rgb, pow(saturate(sd + 0.77), 32.0));
    }

    color.a *= saturate(screenPxDistance + 0.5);
    color.a *= textureColor.a;
    
    return color;
}

[[fragment]]
float4 shaderMain(Interpolators interpolators [[stage_in]],
                  constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                  constant SamplerDescriptorHeap* g_SamplerDescriptorHeap [[buffer(1)]],
                  constant PushConstants& g_PushConstants [[buffer(8)]])
{
    float4 color = interpolators.Color;
    color *= PixelAntialiasing(interpolators.Position.xy - g_PushConstants.ProceduralOrigin, g_PushConstants);
    
    if (g_PushConstants.Texture2DDescriptorIndex != 0)
    {
        texture2d<float> texture = g_Texture2DDescriptorHeap[g_PushConstants.Texture2DDescriptorIndex & 0x7FFFFFFF].tex;
        
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
                float2 fracPart = fract(lowQualityPosition);
                
                float2 uvStep = fwidth(interpolators.UV) * invScale;
                float2 lowQualityUV = interpolators.UV - fracPart * uvStep;
                float2 screenTexSize = 1.0 / uvStep;
                
                float4 topLeft = SampleSdfFont(color, texture, lowQualityUV + float2(0, 0), screenTexSize, g_SamplerDescriptorHeap, g_PushConstants);
                float4 topRight = SampleSdfFont(color, texture, lowQualityUV + float2(uvStep.x, 0), screenTexSize, g_SamplerDescriptorHeap, g_PushConstants);
                float4 bottomLeft = SampleSdfFont(color, texture, lowQualityUV + float2(0, uvStep.y), screenTexSize, g_SamplerDescriptorHeap, g_PushConstants);
                float4 bottomRight = SampleSdfFont(color, texture, lowQualityUV + uvStep.xy, screenTexSize, g_SamplerDescriptorHeap, g_PushConstants);
                
                float4 top = mix(topLeft, topRight, fracPart.x);
                float4 bottom = mix(bottomLeft, bottomRight, fracPart.x);
                
                color = mix(top, bottom, fracPart.y);
            }
            else
            {
                color = SampleSdfFont(color, texture, interpolators.UV, 1.0 / fwidth(interpolators.UV), g_SamplerDescriptorHeap, g_PushConstants);
            }
        }
        else
        {
            color *= texture.sample(g_SamplerDescriptorHeap[0].samp, interpolators.UV);
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

            color.rgb = mix(color.rgb, rimColor, smoothstep(0.0, 1.0, rim));
            color.rgb = mix(color.rgb, shadowColor, smoothstep(0.0, 1.0, shadow));
        }
        else
        {
            float4 top = mix(DecodeColor(g_PushConstants.GradientTopLeft), DecodeColor(g_PushConstants.GradientTopRight), smoothstep(0.0, 1.0, factor.x));
            float4 bottom = mix(DecodeColor(g_PushConstants.GradientBottomLeft), DecodeColor(g_PushConstants.GradientBottomRight), smoothstep(0.0, 1.0, factor.x));
            color *= mix(top, bottom, smoothstep(0.0, 1.0, factor.y));
        }
    }
        
    if (g_PushConstants.ShaderModifier == IMGUI_SHADER_MODIFIER_GRAYSCALE)
        color.rgb = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
    
    return color;
}
