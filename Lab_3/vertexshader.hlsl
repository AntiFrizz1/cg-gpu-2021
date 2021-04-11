#define NUM_OF_LIGHT 3
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 vLightDir[NUM_OF_LIGHT];
    float4 vLightColor[NUM_OF_LIGHT];
    float4 vLightIntensity[NUM_OF_LIGHT];
    float4 Eye;
    float metalness;
    float roughness;
    float3 albedo;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Norm : NORMAL;
    float2 InTexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float4 WorldPos : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    output.WorldPos = mul(input.Pos, world);
    output.Pos = mul(input.Pos, world);
    output.Pos = mul(output.Pos, view);
    output.Pos = mul(output.Pos, projection);
    output.Norm = normalize(mul(input.Norm, (float3x3) world));
    output.TexCoord = input.InTexCoord;
    return output;
}