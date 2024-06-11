SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

cbuffer Params : register(b0)
{
    float light;
    float shift;
    float gamma;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float4 result = color;
    result.rgb = color.rgb * (light + 1);

    result.rgb = pow(result.rgb, (1 / gamma));

    result.rgb = saturate(result.rgb + shift);
    result.a = color.a;

    return result;
}
