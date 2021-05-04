  
Texture2D<float4> sourceTexture : register(t0);

SamplerState samState : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = sourceTexture.Sample(samState, input.Tex);
    float l = 0.2126f * color.r + 0.7151f * color.g + 0.0722f * color.b;
    return log(l + 1);
}