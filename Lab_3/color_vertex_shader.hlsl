struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

PS_INPUT main(uint input : SV_VERTEXID)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.Tex = float2(input & 1, input >> 1);
    output.Pos = float4((output.Tex.x - 0.5f) * 2, -(output.Tex.y - 0.5f) * 2, 0, 1);
    return output;
}