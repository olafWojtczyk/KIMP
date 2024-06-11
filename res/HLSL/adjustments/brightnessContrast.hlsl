SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

cbuffer Params : register(b0)
{
    float vContrast;
    float vBrightness;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);
    float4 result = color;

    //contrast

    float factor = (1.015 * (vContrast / 4 + 1.0)) / (1.0 * (1.015 - vContrast / 4));

    result.rgb = (color.rgb - 0.5) * factor + 0.5;

    //brightness

    result.rgb += vBrightness;
    result.rgb = clamp(result.rgb, 0.0, 1.0);

    return result;
}