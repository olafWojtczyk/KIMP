Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer cb0 : register(b0)
{
    float strength;
    bool monochrome;
}

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float rand(float2 co)
{
    return frac(sin(dot(co, float2(12.9898, 78.233))) * 43758.5453);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float noise = rand(input.TexCoord) * strength;

    if (monochrome)
    {
        color.rgb -= noise;
    }
    else
    {
        color.r -= noise;
        noise = rand(input.TexCoord + float2(1.0, 0.0)) * strength;
        color.g -= noise;
        noise = rand(input.TexCoord + float2(0.0, 1.0)) * strength;
        color.b -= noise;
    }

    color = saturate(color);

    return color;
}