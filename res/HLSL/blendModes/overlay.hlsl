SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //current layer
Texture2D texture1 : register(t1); //accumulative bottom layers

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

float p_overlay(in float base, in float blend) {
    if(base < 0.5) {
        return 2.0*base*blend;
    } else {
        return 1.0 - 2.0 * (1.0 - base) * (1.0 - blend);
    }
}

float4 main(in PSInput input) : SV_TARGET0 {
    float4 base = texture0.Sample(sampler0, input.TexCoord);
    float4 blend = texture1.Sample(sampler0, input.TexCoord);

    return float4(
        p_overlay(base.r, blend.r),
        p_overlay(base.g, blend.g),
        p_overlay(base.b, blend.b),
        base.a
    );
}