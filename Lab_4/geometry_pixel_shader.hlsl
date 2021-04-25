#include "common_shader.fx"

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 n = input.Norm.xyz;
    float3 v = normalize(Eye.xyz - input.WorldPos.xyz);
    float3 color = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < NUM_OF_LIGHT; i++)
    {
        float3 l = normalize(vLightDir[i].xyz - input.WorldPos.xyz);
        float k = pow(roughness + 1, 2) / 8;
        float G = G_func(n, v, l, k) * sign(max(dot(v, n), 0));
        color += G * vLightColor[i].xyz * vLightIntensity[i].x * max(dot(l, n), 0);
    }

    return float4(color, 1.0f);
}