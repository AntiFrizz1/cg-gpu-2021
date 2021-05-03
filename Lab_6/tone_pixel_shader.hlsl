Texture2D<float4> sourceTexture : register(t0);

SamplerState samState : register(s0);

cbuffer AverageLuminanceBuffer : register(b0)
{
    float AverageLuminance;
}

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

float Exposure()
{
    float luminance = AverageLuminance;
    float keyValue = 1.03 - 2 / (2 + log10(luminance + 1));
    return keyValue / luminance;
}

float3 Uncharted2Tonemap(float3 x)
{
    static const float a = 0.1; 
    static const float b = 0.50;
    static const float c = 0.1; 
    static const float d = 0.20; 
    static const float e = 0.02;
    static const float f = 0.30;

    return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

float3 TonemapFilmic(float3 color)
{
    static const float W = 11.2;

    float e = Exposure();
    float3 curr = Uncharted2Tonemap(e * color);
    float3 whiteScale = 1.0f / Uncharted2Tonemap(W);
    return curr * whiteScale;
}

float3 LinearToSRGB(float3 color)
{
    return pow(abs(color), 1 / 2.2f);
}

float4 main(PS_INPUT input) : SV_Target0
{
    float4 color = sourceTexture.Sample(samState, input.Tex);
    return float4(LinearToSRGB(TonemapFilmic(color.xyz)), color.a);
}