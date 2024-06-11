SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

cbuffer Params : register(b0)
{
    float vHue;
    float vSaturation;
    float vValue;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float3 HSVtoRGB(float3 hsv)
{
    float h = hsv.x * 6.0f;
    float s = hsv.y;
    float v = hsv.z;

    int i = (int)floor(h);
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    float3 rgb;
    switch (i)
    {
        case 0: rgb = float3(v, t, p); break;
        case 1: rgb = float3(q, v, p); break;
        case 2: rgb = float3(p, v, t); break;
        case 3: rgb = float3(p, q, v); break;
        case 4: rgb = float3(t, p, v); break;
        default: rgb = float3(v, p, q); break;
    }

    return rgb;
}

float3 RGBtoHSV(float3 rgb)
{
    float r = rgb.r;
    float g = rgb.g;
    float b = rgb.b;

    float maxC = max(r, max(g, b));
    float minC = min(r, min(g, b));
    float delta = maxC - minC;

    float h = 0.0f;
    if (delta > 0.0f)
    {
        if (maxC == r)
        {
            h = (g - b) / delta;
            if (g < b)
                h += 6.0f;
        }
        else if (maxC == g)
        {
            h = (b - r) / delta + 2.0f;
        }
        else
        {
            h = (r - g) / delta + 4.0f;
        }
        h /= 6.0f;
    }

    float s = (maxC == 0.0f) ? 0.0f : delta / maxC;
    float v = maxC;

    return float3(h, s, v);
}

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float3 HSV = RGBtoHSV(color.rgb);

    HSV.x += vHue;

    if (HSV.x > 1.0f) HSV.x -= 1.0f;
    if (HSV.x < 0.0f) HSV.x += 1.0f;

    HSV.y += vSaturation;
    HSV.z += vValue;

    float3 correctedHSV = clamp(HSV, 0.0, 1.0);
    
    return float4(HSVtoRGB(correctedHSV), color.a);
}