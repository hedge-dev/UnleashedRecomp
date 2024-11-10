#include "movie_common.hlsli"

Interpolators main(in VertexShaderInput In)
{
    Interpolators Out;
    Out.ProjPos = In.ObjPos;
    Out.UV = In.UV;
    return Out;
}
