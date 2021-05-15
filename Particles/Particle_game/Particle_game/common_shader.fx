#define NUM_OF_LIGHT 3
#define M_PI 3.14159265358979323846
#define ROUGHNESS_MIN 0.0001

cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 Eye;
};

cbuffer LightsBuffer: register(b1)
{
    float4 vLightDir[NUM_OF_LIGHT];
    float4 vLightColor[NUM_OF_LIGHT];
    float4 vLightIntensity[NUM_OF_LIGHT];
}

cbuffer MaterialBuffer: register(b2)
{
    float4 albedo;
    float metalness;
    float roughness;
}

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
    float value = max(dot(n, h), 0);
    return pow(alpha, 2) / (M_PI * pow(pow(value, 2) * (pow(alpha, 2) - 1) + 1, 2));
}

float G_SCHLICKGGX(float3 n, float3 v, float k)
{
    float value = max(dot(n, v), 0);
    return value / (value * (1 - k) + k);
}

float G_func(float3 n, float3 v, float3 l, float k)
{
    return G_SCHLICKGGX(n, v, k) * G_SCHLICKGGX(n, l, k);
}

float3 Fresnel0()
{
    static const float3 NON_METALNESS_COLOR = float3(0.04, 0.04, 0.04);
    return NON_METALNESS_COLOR * (1 - metalness) + albedo.xyz * metalness;
}


float3 Fresnel(float3 h, float3 v)
{
    float3 F0 = Fresnel0();
    return F0 + (max(1- roughness, F0) - F0) * pow(1 - dot(h, v), 5);
}

/*
float3 Ambient(float3 n, float3 v)
{
    float3 F = Fresnel(n, v);
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metalness;
    float3 irradiance = diffuse_irradiance_map.SampleLevel(samLinear, n, 0).rgb;
    float3 diffuse = irradiance * albedo.xyz;
    float3 ambient = kD * diffuse;

    return float4(ambient, 1.0);
}*/