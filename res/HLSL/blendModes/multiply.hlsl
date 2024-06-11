SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //current layer
Texture2D texture1 : register(t1); //accumulative bottom layers

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float4 main(in PSInput input) : SV_TARGET0 {
    float4 base = texture0.Sample(sampler0, input.TexCoord);
    float4 blend = texture1.Sample(sampler0, input.TexCoord);

    return base * blend;
}