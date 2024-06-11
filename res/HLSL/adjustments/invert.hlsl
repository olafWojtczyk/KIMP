SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    return float4((1.0 - color.rgb), color.a);
}
