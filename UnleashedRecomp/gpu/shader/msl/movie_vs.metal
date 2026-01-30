#include "movie_common.metali"

[[vertex]]
Interpolators shaderMain(VertexShaderInput In [[stage_in]],
                         constant PushConstants& g_PushConstants [[buffer(8)]])
{
    Interpolators Out;
    Out.ProjPos = In.ObjPos;
    Out.ProjPos.xy += g_HalfPixelOffset * Out.ProjPos.w;
    Out.UV = In.UV;
    return Out;
}
