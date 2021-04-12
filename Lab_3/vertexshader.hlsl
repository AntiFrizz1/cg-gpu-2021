#include "commonshader.fx"

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 InTexCoord : TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.WorldPos = mul(input.Pos, world);
    output.Pos = mul(input.Pos, world);
    output.Pos = mul(output.Pos, view);
    output.Pos = mul(output.Pos, projection);
    output.Norm = normalize(mul(input.Norm, (float3x3) world));
    output.TexCoord = input.InTexCoord;
    return output;
}