#include "common_shader.fx"
static const int N1 = 200;
static const int N2 = 50;
TextureCube cubemap_texture : register(t0);


float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.Norm);
    // По умолчанию, "вперед" - это вперед, кроме случая, когда сама нормаль 
    // направлена вперед или назад, тогда "вперед" - это вправо =)
    float3 dir = abs(normal.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);

    float3 tangent = normalize(cross(dir, normal));
    float3 bitangent = cross(normal, tangent);
    float3 irradiance = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < N1; i++)
    {
        for (int j = 0; j < N2; j++)
        {
            float phi = i * (2 * M_PI / N1);
            float theta = j * (M_PI / 2 / N2);
            // Перевод сферических координат в декартовы (в касательном пространстве)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // ... и из касательного пространства в мировое
            float3 sampleVec = tangentSample.x * tangent + tangentSample.y * bitangent + tangentSample.z * normal;
            float3 texture_color = cubemap_texture.SampleLevel(samLinear, sampleVec, 0).xyz;
            //irradiance += cubemap_texture.Sample(samLinear, sampleVec, 0).xyz * cos(theta) * sin(theta);
            irradiance += texture_color * cos(theta) * sin(theta);
        }  
    }
    
    irradiance = M_PI * irradiance / (N1 * N2);
    return float4(irradiance, 1.0f);
}