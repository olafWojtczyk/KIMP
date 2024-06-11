SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0);
Texture2D mask0 : register(t1);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.TexCoord);
    float4 maskColor = mask0.Sample(sampler0, input.TexCoord);

    color.a *= maskColor.r;

    return color;
}