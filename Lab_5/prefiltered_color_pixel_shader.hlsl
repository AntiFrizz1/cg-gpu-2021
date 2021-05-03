#include "common_shader.fx"
#include "common_ibl_shader_func.fx"

static const int SAMPLE_COUNT = 1024;
static const float RESOLUTION = 128;

TextureCube EnvironmentMap : register(t0);


float DistributionGGX(float3 n, float3 h, float roughness)
{
    float roughnessSqr = pow(max(roughness, 0.01f), 2);
    return roughnessSqr / (M_PI * pow(pow(max(dot(n, h), 0), 2) * (roughnessSqr - 1) + 1, 2));
}


float3 PrefilteredColor(float3 norm)
{
    float3 view = norm;
    float totalWeight = 0.0;
    float3 prefilteredColor = float3(0, 0, 0);
    static const uint SAMPLE_COUNT = 1024u;
    for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, norm, roughness);
        float3 L = normalize(2.0 * dot(view, H) * H - view);
        float ndotl = max(dot(norm, L), 0.0);
        float ndoth = max(dot(norm, H), 0.0);
        float hdotv = max(dot(H, view), 0.0);
        float D = DistributionGGX(norm, H, roughness);
        float pdf = (D * ndoth / (4.0 * hdotv)) + 0.0001;
        //float resolution = 512.0; // разрешение иcходной environment map
        float saTexel = 4.0 * M_PI / (6.0 * RESOLUTION * RESOLUTION);
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
        float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
        if (ndotl > 0.0) {
            prefilteredColor += EnvironmentMap.SampleLevel(samLinear, L, mipLevel) * ndotl;
            totalWeight += ndotl;
        }

    }
    prefilteredColor = prefilteredColor / totalWeight;
    return prefilteredColor;
}

float4 main(PS_INPUT input) : SV_TARGET
{

    float3 norm = normalize(input.Norm);

    if (roughness < 0.1f)
        return float4(EnvironmentMap.SampleLevel(samLinear, norm, 0).xyz, 1.0f);
    return float4(PrefilteredColor(norm), 1.0f);

}
