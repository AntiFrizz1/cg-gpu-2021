#include "model_shader_common.fx"

float4 main(PS_INPUT input) : SV_TARGET
{
    return diffuse_texture.Sample(samModel, input.TexCoord);
}