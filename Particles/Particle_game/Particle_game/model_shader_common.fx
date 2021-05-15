#define NUM_OF_LIGHT 3
#define M_PI 3.14159265358979323846
#define ROUGHNESS_MIN 0.0001

TextureCube diffuse_irradiance_map : register(t0);
TextureCube prefiltered_color_texture : register(t1);
Texture2D<float4> preintegrated_brdf_texture : register(t2);

Texture2D<float4> diffuse_texture : register(t3);
Texture2D<float4> metallic_roughness_texture : register(t4);
Texture2D<float4> normal_texture : register(t5);

SamplerState samLinear : register(s0);
SamplerState samLinearClamp : register(s1);
SamplerState samModel : register(s2);

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
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
    float4 Albedo;
    float Metalness;
    float Roughness;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
    float2 TexCoord : TEXCOORD_0;
    float4 WorldPos : POSITION;
#ifdef HAS_TANGENT
    float3 Tangent : TANGENT;
#endif
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

float3 Fresnel0(float3 albedo, float3 metalness)
{
    static const float3 NON_METALNESS_COLOR = float3(0.04, 0.04, 0.04);
    return NON_METALNESS_COLOR * (1 - metalness) + albedo.xyz * metalness;
}


float3 Fresnel(float3 h, float3 v, float3 albedo, float3 metalness, float roughness)
{
    float3 F0 = Fresnel0(albedo, metalness);
    return F0 + (max(1 - roughness, F0) - F0) * pow(1 - dot(h, v), 5);
}

float4 GetAlbedo (float2 uv)
{
#ifdef HAS_COLOR_TEXTURE
    float4 diffuse = diffuse_texture.Sample(samModel, uv);
    float4 albedo = diffuse * Albedo;
#else
    float4 albedo = pow(Albedo, 2.2f);
#endif
#ifdef HAS_OCCLUSION_TEXTURE
    albedo.xyz *= metallic_roughness_texture.Sample(samModel, uv).r;
#endif
    return albedo;
}

float2 GetMetalnessRoughness(float2 uv)
{
    float2 material = float2(Metalness, Roughness);
#ifdef HAS_METAL_ROUGH_TEXTURE
    material *= metallic_roughness_texture.Sample(samModel, uv).bg;
#endif
    return material.xy;
}