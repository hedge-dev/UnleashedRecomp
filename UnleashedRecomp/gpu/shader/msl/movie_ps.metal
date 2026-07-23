#include "movie_common.metali"

[[fragment]]
PixelShaderOutput shaderMain(Interpolators In [[stage_in]],
                             constant Texture2DDescriptorHeap* g_Texture2DDescriptorHeap [[buffer(0)]],
                             constant SamplerDescriptorHeap* g_SamplerDescriptorHeap [[buffer(3)]],
                             constant PushConstants& g_PushConstants [[buffer(8)]])
{
    texture2d<float> Tex0 = g_Texture2DDescriptorHeap[Tex0_ResourceDescriptorIndex].tex;
    texture2d<float> Tex1 = g_Texture2DDescriptorHeap[Tex1_ResourceDescriptorIndex].tex;
    texture2d<float> Tex2 = g_Texture2DDescriptorHeap[Tex2_ResourceDescriptorIndex].tex;
    texture2d<float> Tex3 = g_Texture2DDescriptorHeap[Tex3_ResourceDescriptorIndex].tex;
    texture2d<float> Tex4 = g_Texture2DDescriptorHeap[Tex4_ResourceDescriptorIndex].tex;
    
    sampler Tex0_s = g_SamplerDescriptorHeap[Tex0_SamplerDescriptorIndex].samp;
    sampler Tex1_s = g_SamplerDescriptorHeap[Tex1_SamplerDescriptorIndex].samp;
    sampler Tex2_s = g_SamplerDescriptorHeap[Tex2_SamplerDescriptorIndex].samp;
    sampler Tex3_s = g_SamplerDescriptorHeap[Tex3_SamplerDescriptorIndex].samp;
    sampler Tex4_s = g_SamplerDescriptorHeap[Tex4_SamplerDescriptorIndex].samp;
    
    PixelShaderOutput Out;
    float ValY = Tex0.sample(Tex0_s, In.UV).r;
    float ValU = Tex1.sample(Tex1_s, In.UV).r - 0.5;
    float ValV = Tex2.sample(Tex2_s, In.UV).r - 0.5;
    float ValA = 1.0;
    float ValD = 0.0;
    if (bAmv)
        ValA = (Tex3.sample(Tex3_s, In.UV).r - 0.0625) * 1.164;
    if (bZmv)
    {
        ValD = (Tex4.sample(Tex4_s, In.UV).r - 0.0625) * 1.164;
        if (ValD < 9.0 / 255.0)
        {
            ValD = 0.0;
        }
        else if (ValD < 17.0 / 255.0)
        {
            ValD = fZmin;
        }
        else if (ValD < 224.0 / 255.0)
        {
            ValD = (ValD - 17.0 / 255.0) / (223.0 / 255.0 - 17.0 / 255.0) * (fZmax - fZmin) + fZmin;
        }
        else if (ValD < 240.0 / 255.0)
        {
            ValD = fZmax;
        }
        else
        {
            ValD = 1.0;
        }
    }
    if (bCsc)
    {
        if (ValY < 16.0 / 255.0)
        {
            ValY = ValY * 3.0 / 2.0;
        }
        else if (ValY < 176.0 / 255.0)
        {
            ValY = 24.0 / 255.0 + (ValY - 16.0 / 255.0) / 2.0;
        }
        else if (ValY < 192.0 / 255.0)
        {
            ValY = 104.0 / 255.0 + (ValY - 176.0 / 255.0) / 1.0;
        }
        else
        {
            ValY = 120.0 / 255.0 + (ValY - 192.0 / 255.0) * 2.0;
        }
        if (abs(ValU) < 24.0 / 255.0)
        {
            ValU /= 3.0;
        }
        else
        {
            ValU = (8.0 / 255.0 + (abs(ValU) - 24.0 / 255.0) * (120.0 / 104.0)) * sign(ValU);
        }
        if (abs(ValV) < 24.0 / 255.0)
        {
            ValV /= 3.0;
        }
        else
        {
            ValV = (8.0 / 255.0 + (abs(ValV) - 24.0 / 255.0) * (120.0 / 104.0)) * sign(ValV);
        }
        Out.Color.r = ValY + ValV * 1.402;
        Out.Color.g = ValY - ValU * 0.344 - ValV * 0.714;
        Out.Color.b = ValY + ValU * 1.772;
    }
    else
    {
        ValY = (ValY - 0.0625) * 1.164;
        Out.Color.r = ValY + ValV * 1.596;
        Out.Color.g = ValY - ValU * 0.392 - ValV * 0.813;
        Out.Color.b = ValY + ValU * 2.017;
    }
    Out.Color.a = ValA;
    
    if (any(In.UV < 0.0) || any(In.UV > 1.0))
        Out.Color.rgb = 0.0;

    return Out;
}
