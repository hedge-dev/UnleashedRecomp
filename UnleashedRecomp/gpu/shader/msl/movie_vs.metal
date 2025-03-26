#include "movie_common.metali"

[[vertex]]
Interpolators shaderMain(VertexShaderInput In [[stage_in]])
{
    Interpolators Out;
    Out.ProjPos = In.ObjPos;
    Out.UV = In.UV;
    return Out;
}
