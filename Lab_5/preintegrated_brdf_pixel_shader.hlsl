#include "common_shader.fx"
#include "common_ibl_shader_func.fx"


float SchlickGGX(float3 n, float3 v, float k)
{
    float value = max(dot(n, v), 0);
    return value / (value * (1 - k) + k);
}

float GeometrySmith(float3 n, float3 v, float3 l, float roughness)
{
    float k = pow(roughness, 2) / 2;
    return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}


float2 IntegrateBRDF(float NdotV, float roughness)
{
    // Вычисляем V по NdotV, считая, что N=(0,1,0)
    float3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.z = 0.0;
    V.y = NdotV;
    // Кэффициенты, которые хотим посчитать
    float A = 0.0;
    float B = 0.0;
    float3 N = float3(0.0, 1.0, 0.0);
    // Генерация псевдослучайных H
    static const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);
        float NdotL = max(L.y, 0.0);
        float NdotH = max(H.y, 0.0);
        float VdotH = max(dot(V, H), 0.0);
        if (NdotL > 0.0) {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return float2(A, B);
}



float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(IntegrateBRDF(input.Norm.x, 1 - input.Norm.y), 0.0f, 1.0f);
}