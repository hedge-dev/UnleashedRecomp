#include "imgui_common.hlsli"

void main(in float2 position : POSITION, in float2 uv : TEXCOORD, in float4 color : COLOR, out Interpolators interpolators)
{
    interpolators.Position = float4(position * g_PushConstants.InverseDisplaySize * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    interpolators.UV = uv;
    interpolators.Color = color;
}
