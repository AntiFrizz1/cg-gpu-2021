#include "model_shader_common.fx"

static const float MAX_REFLECTION_LOD = 4.0;

float3 Ambient(float3 n, float3 v, float roughness, float metalness, float4 albedo)
{
    float3 r = normalize(reflect(-v, n));
    float3 prefilteredColor = prefiltered_color_texture.SampleLevel(samLinear, r, roughness * MAX_REFLECTION_LOD);
    float3 F0 = Fresnel0(metalness, albedo);
    float3 F = Fresnel(n, v, roughness, metalness, albedo);
    float2 envBRDF = preintegrated_brdf_texture.Sample(samLinearClamp, float2(max(dot(n, v), 0.0), roughness));
    float3 specular = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metalness;
    float3 irradiance = diffuse_irradiance_map.Sample(samLinear, n);
    float3 diffuse = irradiance * albedo.xyz;
    float3 ambient = (kD * diffuse + specular);
    return ambient;
}


float3 BRDF(float3 n, float3 v, float3 l, float roughness, float metalness, float4 albedo)
{
    float3 color;
    float3 h = normalize(v + l);
    float k = pow(roughness + 1, 2) / 8;
    float G = G_func(n, v, l, k) * sign(max(dot(v, n), 0));
    float D = NDG_GGXTR(n, h, roughness) * sign(max(dot(l, n), 0));
    float3 F = Fresnel(h, v, roughness, metalness, albedo) * sign(max(dot(l, n), 0));

    color = (1 - F) * albedo.xyz / M_PI * (1 - metalness) + D * F * G / (ROUGHNESS_MIN + 4 * (max(dot(l, n), 0) * max(dot(v, n), 0)));
    return float4(color, 1.0);
}

float4 GetAlbedo(float2 uv)
{
#ifdef BASE_COLOR_TEXTURE
float4 diffuse = diffuse_texture.Sample(samModel, uv);
float4 albedo = diffuse * Albedo;
#else
    float4 albedo = pow(Albedo, 2.2f);
#endif
#ifdef OCCLUSION_TEXTURE
albedo.xyz *= oclusion_texture.Sample(samModel, uv).r;
#endif
    return albedo;
}

float2 GetMetalnessRoughness(float2 uv)
{
    float2 material = float2(Metalness, Roughness);
#ifdef METALLIC_ROUGHNESS_TEXTURE
material *= metallic_roughness_texture.Sample(samModel, uv).bg;
#endif
    return material.xy;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

#ifdef NORMAL_TEXTURE
float3 n_model = (normal_texture.Sample(samModel, input.TexCoord) * 2.0f - 1.0f).xyz;
float3 tangent = input.Tangent;
if (length(input.Tangent) > 0)
tangent = normalize(input.Tangent);
float3 binormal = cross(n, tangent);
n = normalize(n_model.x * tangent + n_model.y * binormal + n);
#endif

    float2 metalness_roughness = GetMetalnessRoughness(input.TexCoord);
    float metalness = metalness_roughness.x;
    float roughness = metalness_roughness.y;
    float4 albedo = GetAlbedo(input.TexCoord);

    for (int i = 0; i < NUM_OF_LIGHT; i++)
    {
        float3 l = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
        float3 LO_i = BRDF(n, v, l, roughness, metalness, albedo) * vLightColor[i].xyz * vLightIntensity[i].x * max(dot(l, n), 0);
        color += LO_i;
    }
    color += Ambient(n, v, roughness, metalness, albedo).xyz;
    return float4(color, albedo.a);
}
