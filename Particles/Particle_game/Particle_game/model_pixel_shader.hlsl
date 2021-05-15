#include "model_shader_common.fx"

static const float MAX_REFLECTION_LOD = 4.0;

float3 Ambient(float3 n, float3 v, float3 albedo, float3 metalness, float roughness)
{
    float3 r = normalize(reflect(-v, n));
    float3 prefilteredColor = prefiltered_color_texture.SampleLevel(samLinear, r, roughness * MAX_REFLECTION_LOD);
    float3 F0 = Fresnel0(albedo, metalness);
    float3 F = Fresnel(n, v, albedo, metalness, roughness);
    float2 envBRDF = preintegrated_brdf_texture.Sample(samLinearClamp, float2(max(dot(n, v), 0.0), roughness));
    float3 specular = prefilteredColor * (F0 * envBRDF.x + envBRDF.y);

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metalness;
    float3 irradiance = diffuse_irradiance_map.Sample(samLinear, n);
    float3 diffuse = irradiance * albedo;
    float3 ambient = (kD * diffuse + specular);
    return ambient;
}


float3 BRDF(float3 n, float3 v, float3 l, float3 albedo, float3 metalness, float roughness)
{
    float3 color;
    float3 h = normalize(v + l);
    float k = pow(roughness + 1, 2) / 8;
    float G = G_func(n, v, l, k) * sign(max(dot(v, n), 0));
    float D = NDG_GGXTR(n, h, roughness) * sign(max(dot(l, n), 0));
    float3 F = Fresnel(h, v, albedo, metalness, roughness) * sign(max(dot(l, n), 0));

    color = (1 - F) * albedo.xyz / M_PI * (1 - metalness) + D * F * G / (ROUGHNESS_MIN + 4 * (max(dot(l, n), 0) * max(dot(v, n), 0)));
    return float4(color, 1.0);
}


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

#ifdef HAS_NORMAL_TEXTURE
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
        float3 LO_i = BRDF(n, v, l, albedo, metalness, roughness) * vLightColor[i].xyz * vLightIntensity[i].x * max(dot(l, n), 0);
        color += LO_i;
    }
    color += Ambient(n, v, albedo, metalness, roughness).xyz;
    return float4(color, albedo.a);
}