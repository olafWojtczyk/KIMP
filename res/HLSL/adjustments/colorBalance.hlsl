SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0); // affected layer

cbuffer Params : register(b0)
{
    float3 shadows;
    float3 midtones;
    float3 highlights;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float3 applyColorBalance(float3 color, float3 adjustment, float weight)
{
    return lerp(color, color + adjustment, weight);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));

    float shadowWeight = saturate((0.5 - luminance) * 2.0);
    float highlightWeight = saturate((luminance - 0.5) * 2.0);
    float midtoneWeight = 1.0 - shadowWeight - highlightWeight;

    float3 adjustedColor = applyColorBalance(color.rgb, shadows, shadowWeight);
    adjustedColor = applyColorBalance(adjustedColor, midtones, midtoneWeight);
    adjustedColor = applyColorBalance(adjustedColor, highlights, highlightWeight);

    // Clamp final color to the range [0, 1]
    color.rgb = clamp(adjustedColor, 0.0, 1.0);

    return color;
}
