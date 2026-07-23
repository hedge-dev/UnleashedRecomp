void shaderMain(in uint vertexId : SV_VertexID, out float4 position : SV_Position, out float2 texCoord : TEXCOORD)
{
    texCoord = float2((vertexId << 1) & 2, vertexId & 2);
    position = float4(texCoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
}
