#define NUM_OF_LIGHT 3
#define M_PI           3.14159265358979323846
#define ROUGHNESS_MIN 0.0001

const float3 NON_METALNESS_COLOR = float3(0.04, 0.04, 0.04);
cbuffer ConstantBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 vLightDir[NUM_OF_LIGHT];
    float4 vLightColor[NUM_OF_LIGHT];
    float4 vLightIntensity[NUM_OF_LIGHT];
    float4 Eye;
    float4 albedo;
    float metalness;
    float roughness;
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
    float value = max(dot(n, h), 0);
    return pow(alpha, 2) / (M_PI * pow(pow(value, 2) * (pow(alpha, 2) - 1) + 1, 2));
}

float G_SCHLICKGGX(float3 n, float3 v, float k)
{
    return dot(n, v) / (dot(n, v) * (1 - k) + k);
}

float G_func(float3 n, float3 v, float3 l, float k)
{
    return G_SCHLICKGGX(n, v, k) * G_SCHLICKGGX(n, l, k);
}

float3 Fresnel0()
{
    return  NON_METALNESS_COLOR * (1 - metalness) + albedo.xyz * metalness;
}


float3 Fresnel(float3 h, float3 v)
{
    float3 F0 = Fresnel0();
    return F0 + (1 - F0) * pow(1 - dot(h, v), 5);
}

float3 BRDF(float3 n, float3 v, float3 l) 
{
    float3 h = normalize(v + l);
    float k = pow(roughness + 1, 2) / 8;
    float G = G_func(n, v, l, k) * sign(max(dot(v, n), 0));
    float D = NDG_GGXTR(n, h, roughness) * sign(max(dot(l, n), 0));
    float3 F = Fresnel(h, v) *sign(max(dot(l, n), 0));
    return (1 - F) * albedo.xyz / M_PI * (1 - metalness) + D * F * G / (ROUGHNESS_MIN + 4 * (max(dot(l, n), 0) * max(dot(v, n), 0)));
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < NUM_OF_LIGHT; i++) 
    {   
        float3 l = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
        float3 LO_i = BRDF(n, v, l) * vLightColor[i].xyz * vLightIntensity[i].x * max(dot(l, n), 0);
        color += LO_i;
    }

    return float4(color, 1.0f);
}