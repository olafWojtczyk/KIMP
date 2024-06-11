Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer cb0 : register(b0)
{
    float strength;
    float directionX;
    float directionY;
    float x;
    float y;
}

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

static const float weights[5] = { 0.36918, 0.31725, 0.19823, 0.08812, 0.02643 };

float4 main(PSInput input) : SV_TARGET
{
    float2 texelSize = (1.f + (strength - 1.f) * 3.5f) / float2(x, y);
    float4 color = float4(0, 0, 0, 0);

    int j = 0;

    for (int i = -2; i <= 2; ++i)
    {
        float2 offset = float2(i, i) * texelSize;
        offset *= float2(directionX, directionY);
        color += texture0.Sample(sampler0, input.TexCoord + offset + texelSize * float2(directionX, directionY)) * weights[j];

        j++;
    }

    float4 original = texture0.Sample(sampler0, input.TexCoord);

    color.a = original.a;

    return saturate(color);
}