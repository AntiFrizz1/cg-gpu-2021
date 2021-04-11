#define NUM_OF_LIGHT 3
#define M_PI           3.14159265358979323846
#define ROUGHNESS_MIN 0.0001
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

float NDG_GGXTR(float3 n, float3 h, float alpha)
{
    alpha = max(alpha, ROUGHNESS_MIN);
    return pow(alpha, 2) / (M_PI * pow(pow(dot(n, h), 2) * (pow(alpha, 2) - 1) + 1, 2));
}

float G_SCHLICKGGX(float3 n, float3 v, float k)
{
    return dot(n, v) / (dot(n, v) * (1 - k) + k);
}

float G(float3 n, float3 v, float3 l, float k)
{
    return G_SCHLICKGGX(n, v, k) * G_SCHLICKGGX(n, l, k);
}

float3 Fresnel0()
{
    return float3(0.04, 0.04, 0.04) * (1 - metalness) + albedo * metalness;
}

float3 Fresnel(float3 h, float3 v)
{
    float3 F0 = Fresnel0();
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow(1 - dot(h, v), 5);
}

float4 main(PS_INPUT input) : SV_TARGET 
{
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 l = normalize(vLightDir[0].xyz - input.WorldPos.xyz);
    float3 h = normalize((v + l) / 2);
    float k = pow(roughness + 1, 2) / 8;
    float3 c = float3(0.0f, 0.0f, 0.0f);
    float3 n = input.Norm.xyz;
    float g = G(n, v, l, k);
    float ndg = NDG_GGXTR(n, h, roughness);
    float3 fresnel = Fresnel(h, v);
    if (dot(l, n) >= 0 && dot(v, n) >= 0)
    {
        c = fresnel;
    }
    
    return float4(c, 1.0f);
}