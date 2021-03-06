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

Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Norm : TEXCOORD0;
	float2 TexCoord : TEXCOORD1;
	float4 WorldPos : TEXCOORD2;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    //float4 finalColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    //for (int i = 0; i < NUM_OF_LIGHT; i++)
    //{
    //    float intensity = vLightIntensity[i].x;
    //    float4 curColor = float4(0.0f, 0.0f, 0.0f, vLightColor[i].a);
    //    float specTerm = 0;
    //    float3 dirToLight = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
    //    float diffuseTerm = pow(saturate(dot(dirToLight, input.Norm)), 100);
    //    curColor = diffuseTerm * vLightColor[i] * intensity;
    //
    //    finalColor += curColor;
    //}
    
   // return finalColor;
	return txDiffuse.Sample(samLinear, input.TexCoord);
}