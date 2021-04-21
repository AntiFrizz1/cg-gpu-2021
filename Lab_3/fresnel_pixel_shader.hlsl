#include "common_shader.fx"

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < NUM_OF_LIGHT; i++)
    {
        float3 l = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
        float3 h = normalize(v + l);
        float3 F = Fresnel(h, v) * sign(max(dot(l, n), 0));
        color += F * vLightColor[i].xyz * vLightIntensity[i].x;
    }

    return float4(color, 1.0f);
}