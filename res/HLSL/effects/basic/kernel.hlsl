Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer cb0 : register(b0)
{
    float k1x1;
    float k1x2;
    float k1x3;
    float k2x1;
    float k2x2;
    float k2x3;
    float k3x1;
    float k3x2;
    float k3x3;
    float denominator;
    float shift;
    float x;
    float y;
    float amp;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.TexCoord);
    float3x3 kernel = float3x3(
        float3(k1x1, k1x2, k1x3),
        float3(k2x1, k2x2, k2x3),
        float3(k3x1, k3x2, k3x3)
    );

   float2 texelSize = amp / float2(x, y);
   float3 sum = float3(0.0f, 0.0f, 0.0f);
   [unroll]
    for (int i = -1; i <= 1; i++)
    {
        [unroll]
        for (int j = -1; j <= 1; j++)
        {
            float2 offset = clamp(input.TexCoord + float2(j, i) * texelSize, 0.f, 1.f);
            sum += texture0.Sample(sampler0, offset).rgb * kernel[j + 1][i + 1];
        }
    }

    sum /= denominator;
    sum += shift;

    sum = clamp(sum, 0.f, 1.f);

    return float4(sum.rgb, color.a);
}