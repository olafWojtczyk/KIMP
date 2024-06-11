SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //current layer
Texture2D texture1 : register(t1); //accumulative bottom layers

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float p_softLight(in float base, in float blend) {
    if(blend < 0.5) {
        return 2.0 * base * blend + base * base * (1.0 - 2.0 * blend);
    } else {
        return 2.0 * base * (1.0 - blend) + sqrt(base) * (2.0 * blend - 1.0);
    }
}

float4 main(in PSInput input) : SV_TARGET0 {
    float4 base = texture0.Sample(sampler0, input.TexCoord);
    float4 blend = texture1.Sample(sampler0, input.TexCoord);

    return float4(
        p_softLight(base.r, blend.r),
        p_softLight(base.g, blend.g),
        p_softLight(base.b, blend.b),
        base.a
    );
}