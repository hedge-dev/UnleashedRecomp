struct Interpolators
{
    float4 position [[position]];
    float2 texCoord [[user(TEXCOORD)]];
};

[[vertex]]
Interpolators shaderMain(uint vertexId [[vertex_id]])
{
    Interpolators interpolators;

    interpolators.texCoord = float2((vertexId << 1) & 2, vertexId & 2);
    interpolators.position = float4(interpolators.texCoord * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);

    return interpolators;
}
