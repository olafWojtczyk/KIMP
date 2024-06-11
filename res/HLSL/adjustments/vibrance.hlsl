SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0); //affected layer

cbuffer Params : register(b0)
{
    float vSaturation;
    float vVibrance;
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float3 HSLToRGB(float3 hsl)
{
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    float r, g, b;

    if (s == 0.0)
    {
        r = g = b = l; // achromatic
    }
    else
    {
        float q = (l < 0.5) ? l * (1.0 + s) : l + s - l * s;
        float p = 2.0 * l - q;
        float3 rgb;
        rgb.x = h + 1.0 / 3.0;
        rgb.y = h;
        rgb.z = h - 1.0 / 3.0;

        for (int i = 0; i < 3; i++)
        {
            float t = rgb[i];
            if (t < 0.0) t += 1.0;
            if (t > 1.0) t -= 1.0;
            if (t < 1.0 / 6.0) rgb[i] = p + (q - p) * 6.0 * t;
            else if (t < 1.0 / 2.0) rgb[i] = q;
            else if (t < 2.0 / 3.0) rgb[i] = p + (q - p) * (2.0 / 3.0 - t) * 6.0;
            else rgb[i] = p;
        }

        r = rgb.x;
        g = rgb.y;
        b = rgb.z;
    }

    return float3(r, g, b);
}

float3 RGBToHSL(float3 color)
{
    float r = color.r;
    float g = color.g;
    float b = color.b;

    float maxColor = max(max(r, g), b);
    float minColor = min(min(r, g), b);

    float l = (maxColor + minColor) / 2.0;
    float s = 0.0;
    float h = 0.0;

    if (maxColor != minColor)
    {
        float delta = maxColor - minColor;

        s = (l > 0.5) ? delta / (2.0 - maxColor - minColor) : delta / (maxColor + minColor);

        if (maxColor == r)
            h = (g - b) / delta + (g < b ? 6.0 : 0.0);
        else if (maxColor == g)
            h = (b - r) / delta + 2.0;
        else
            h = (r - g) / delta + 4.0;

        h /= 6.0;
    }

    return float3(h, s, l);
}

float3 ApplyVibrance(float3 color, float vibrance)
{
    float3 hsl = RGBToHSL(color);

    float saturation = hsl.y;
    float luminance = hsl.z;

    float boost = saturate(1.0 - saturation) * vibrance / 5.0f;

    hsl.y += boost * (1.0 - luminance);

    hsl.y = saturate(hsl.y + vSaturation);

    return HSLToRGB(hsl);
}

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);

    float3 returnRGB = color.rgb;
    returnRGB = ApplyVibrance(returnRGB, vVibrance);

    return float4(returnRGB, color.a);
}
