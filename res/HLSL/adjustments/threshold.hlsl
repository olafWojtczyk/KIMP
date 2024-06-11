SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0); // affected layer

cbuffer Params : register(b0)
{
    float level;
};

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));

    float thresholdedValue = step(level, luminance);

    return float4(thresholdedValue, thresholdedValue, thresholdedValue, color.a);
}