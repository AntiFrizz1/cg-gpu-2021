#include "model_shader_common.fx"

struct VS_INPUT
{

    float3 Norm : NORMAL;
    float4 Pos : POSITION;
#ifdef HAS_TANGENT
    float4 Tangent : TANGENT;
#endif
    float2 InTexCoord : TEXCOORD_0;
};


PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.WorldPos = mul(input.Pos, World);
    output.Pos = mul(input.Pos, World);
    output.Pos = mul(output.Pos, View);
    output.Pos = mul(output.Pos, Projection);
    output.Norm = normalize(mul(input.Norm, (float3x3) World));
    output.TexCoord = input.InTexCoord;
#ifdef HAS_TANGENT
    output.Tangent = input.Tangent.xyz;
    if (length(input.Tangent.xyz) > 0)
        output.Tangent = normalize(mul(input.Tangent.xyz, (float3x3)World));
#endif
    return output;
}
