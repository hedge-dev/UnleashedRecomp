#include "../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#ifdef __spirv__

#define g_ViewportSize vk::RawBufferLoad<float4>(g_PushConstants.VertexShaderConstants + 2880, 0x10)
#define g_Z vk::RawBufferLoad<float4>(g_PushConstants.VertexShaderConstants + 3936, 0x10)

#else

cbuffer VertexShaderConstants : register(b0, space4)
{
    float4 g_ViewportSize : packoffset(c180);
    float4 g_Z : packoffset(c246);
};

cbuffer SharedConstants : register(b2, space4)
{
	DEFINE_SHARED_CONSTANTS();
};

#endif

void main(
	[[vk::location(0)]] in float4 iPosition0 : POSITION0,
	[[vk::location(8)]] in float4 iColor0 : COLOR0,
	out float4 oPos : SV_Position,
	out float4 oTexCoord0 : TEXCOORD0,
	out float4 oTexCoord1 : TEXCOORD1,
	out float4 oTexCoord2 : TEXCOORD2,
	out float4 oTexCoord3 : TEXCOORD3,
	out float4 oTexCoord4 : TEXCOORD4,
	out float4 oTexCoord5 : TEXCOORD5,
	out float4 oTexCoord6 : TEXCOORD6,
	out float4 oTexCoord7 : TEXCOORD7,
	out float4 oTexCoord8 : TEXCOORD8,
	out float4 oTexCoord9 : TEXCOORD9,
	out float4 oTexCoord10 : TEXCOORD10,
	out float4 oTexCoord11 : TEXCOORD11,
	out float4 oTexCoord12 : TEXCOORD12,
	out float4 oTexCoord13 : TEXCOORD13,
	out float4 oTexCoord14 : TEXCOORD14,
	out float4 oTexCoord15 : TEXCOORD15,
	out float4 oColor0 : COLOR0,
	out float4 oColor1 : COLOR1)
{
    oPos.xy = (iPosition0.xy - 0.5) * g_ViewportSize.zw * float2(2.0, -2.0) + float2(-1.0, 1.0);
    oPos.z = g_Z.x;
    oPos.w = 1.0;
    oTexCoord0 = iColor0.wxyz;
    oTexCoord1 = 0.0;
    oTexCoord2 = 0.0;
    oTexCoord3 = 0.0;
    oTexCoord4 = 0.0;
    oTexCoord5 = 0.0;
    oTexCoord6 = 0.0;
    oTexCoord7 = 0.0;
    oTexCoord8 = 0.0;
    oTexCoord9 = 0.0;
    oTexCoord10 = 0.0;
    oTexCoord11 = 0.0;
    oTexCoord12 = 0.0;
    oTexCoord13 = 0.0;
    oTexCoord14 = 0.0;
    oTexCoord15 = 0.0;
    oColor0 = 0.0;
    oColor1 = 0.0;
}
