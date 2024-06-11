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
    
    float scale = 1.0 / level;

    color.r = floor(color.r / scale + 0.5) * scale;
    color.g = floor(color.g / scale + 0.5) * scale;
    color.b = floor(color.b / scale + 0.5) * scale;

    return color;
}