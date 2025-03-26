#include "../../../../tools/XenosRecomp/XenosRecomp/shader_common.h"

#define g_ViewportSize (*(reinterpret_cast<device float4*>(g_PushConstants.VertexShaderConstants + 2880)))
#define g_Z (*(reinterpret_cast<device float4*>(g_PushConstants.VertexShaderConstants + 3936)))

struct VertexShaderInput
{
    float4 iPosition0 [[attribute(0)]];
    float4 iColor0 [[attribute(8)]];
};

struct Interpolators
{
    float4 oPos [[position]];
    float4 oTexCoord0;
    float4 oTexCoord1;
    float4 oTexCoord2;
    float4 oTexCoord3;
    float4 oTexCoord4;
    float4 oTexCoord5;
    float4 oTexCoord6;
    float4 oTexCoord7;
    float4 oTexCoord8;
    float4 oTexCoord9;
    float4 oTexCoord10;
    float4 oTexCoord11;
    float4 oTexCoord12;
    float4 oTexCoord13;
    float4 oTexCoord14;
    float4 oTexCoord15;
    float4 oColor0;
    float4 oColor1;
};

[[vertex]]
Interpolators shaderMain(VertexShaderInput input [[stage_in]],
                         constant PushConstants& g_PushConstants [[buffer(8)]])
{
    Interpolators output;

    output.oPos.xy = (input.iPosition0.xy - 0.5) * g_ViewportSize.zw * float2(2.0, -2.0) + float2(-1.0, 1.0);
    output.oPos.z = g_Z.x;
    output.oPos.w = 1.0;
    output.oTexCoord0 = input.iColor0.wxyz;
    output.oTexCoord1 = 0.0;
    output.oTexCoord2 = 0.0;
    output.oTexCoord3 = 0.0;
    output.oTexCoord4 = 0.0;
    output.oTexCoord5 = 0.0;
    output.oTexCoord6 = 0.0;
    output.oTexCoord7 = 0.0;
    output.oTexCoord8 = 0.0;
    output.oTexCoord9 = 0.0;
    output.oTexCoord10 = 0.0;
    output.oTexCoord11 = 0.0;
    output.oTexCoord12 = 0.0;
    output.oTexCoord13 = 0.0;
    output.oTexCoord14 = 0.0;
    output.oTexCoord15 = 0.0;
    output.oColor0 = 0.0;
    output.oColor1 = 0.0;

    return output;
}
