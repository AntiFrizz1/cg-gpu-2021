#include "common_shader.fx"


//SamplerState samLinear : register(s0);
Texture2D<float4> envSphereTexture : register (t0);


float4 main(PS_INPUT input) : SV_TARGET
{
    //return float4(1.0, 1.0, 0.0, 1.0);
    float3 pos = normalize(input.Norm);
    float u = 1.0f - atan2(pos.z, pos.x) / (2 * M_PI);
    float v = 0.5f - asin(pos.y) / M_PI;
    return float4(envSphereTexture.Sample(samLinear, float2(u, v)).xyz, 1.0f);
}