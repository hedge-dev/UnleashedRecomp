#include "../../../thirdparty/ShaderRecomp/ShaderRecomp/shader_common.hlsli"

#ifdef __spirv__

#define g_Gamma vk::RawBufferLoad<float3>(g_PushConstants.SharedConstants + 0)
#define g_TextureDescriptorIndex vk::RawBufferLoad<uint>(g_PushConstants.SharedConstants + 12)

#else

cbuffer SharedConstants : register(b2, space4)
{
    float3 g_Gamma : packoffset(c0.x);
    uint g_TextureDescriptorIndex : packoffset(c0.w);
};

#endif

float4 main(in float4 position : SV_Position) : SV_Target
{
    Texture2D<float4> texture = g_Texture2DDescriptorHeap[g_TextureDescriptorIndex];
    float4 color = texture.Load(int3(position.xy, 0));
    color.rgb = pow(color.rgb, g_Gamma);
    return color;
}
