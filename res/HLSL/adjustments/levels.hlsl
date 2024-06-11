SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

cbuffer Params : register(b0)
{
    float RGBblackPoint;
    float RGBmidtone;
    float RGBwhitePoint;
    float RGBbThreshold;
    float RGBtThreshold;

    float RblackPoint;
    float Rmidtone;
    float RwhitePoint;
    float RbThreshold;
    float RtThreshold;

    float GblackPoint;
    float Gmidtone;
    float GwhitePoint;
    float GbThreshold;
    float GtThreshold;

    float BblackPoint;
    float Bmidtone;
    float BwhitePoint;
    float BbThreshold;
    float BtThreshold;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float AdjustChannel(float color, float blackPoint, float midtone, float whitePoint, float bThreshold, float tThreshold)
{
    float normalized = clamp(color, blackPoint, whitePoint);

    normalized = saturate(normalized);

    float adjusted = pow(normalized, midtone);

    float rescaled = clamp(adjusted, bThreshold, tThreshold);

    return rescaled;
}

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    color.r = AdjustChannel(color.r, RblackPoint, Rmidtone, RwhitePoint, RbThreshold, RtThreshold);
    color.g = AdjustChannel(color.g, GblackPoint, Gmidtone, GwhitePoint, GbThreshold, GtThreshold);
    color.b = AdjustChannel(color.b, BblackPoint, Bmidtone, BwhitePoint, BbThreshold, BtThreshold);

    color.rgb = float3(
        AdjustChannel(color.r, RGBblackPoint, RGBmidtone, RGBwhitePoint, RGBbThreshold, RGBtThreshold),
        AdjustChannel(color.g, RGBblackPoint, RGBmidtone, RGBwhitePoint, RGBbThreshold, RGBtThreshold),
        AdjustChannel(color.b, RGBblackPoint, RGBmidtone, RGBwhitePoint, RGBbThreshold, RGBtThreshold)
    );

    return color;
}

