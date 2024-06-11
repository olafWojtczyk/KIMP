SamplerState sampler0 : register(s0);

Texture2D texture0 : register(t0);
Texture2D heightMap : register(t1);

cbuffer Params : register(b0)
{
    int n;
    int channels;
}

struct Point
{
    float x;
    float y;
};

void setPoints(Point Points[256]) {
    for (int i = 0; i < n; i++) {
        float4 color = heightMap.Sample(sampler0, float2(i / (float)n, 0));
        Points[i].x = color.r;
        Points[i].y = color.g;
    }
}

float LagrangePolynomial(float x, Point Points[256]) {
    float result = 0.0;
    if(n == 0) {
        return x;
    } else {
        for (int i = 0; i < n; i++) {
            float xi = Points[i].x;
            float yi = Points[i].y;
            float li = 1.0;

            for (int j = 0; j < n; ++j) {
                if (i != j) {
                    float xj = Points[j].x;
                    li *= (x - xj) / (xi - xj);
                }
            }
            result += li * yi;
        }
        return result;
    }
}

struct PSInput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET {
    float4 color = texture0.Sample(sampler0, input.TexCoord);
    float4 result = color;

    Point Points[256];
    for(int i = 0; i < 256; ++i) {
        Points[i].x = 0.0f;
        Points[i].y = 0.0f;
    }

    setPoints(Points);

    if (channels == 0) { //RGB
        result.rgb = float3(
            LagrangePolynomial(color.r, Points),
            LagrangePolynomial(color.g, Points),
            LagrangePolynomial(color.b, Points)
        );
    } else if (channels == 1) { //R
        result.r = LagrangePolynomial(color.r, Points);
    } else if (channels == 2) { //G
        result.g = LagrangePolynomial(color.g, Points);
    } else { //B
        result.b = LagrangePolynomial(color.b, Points);
    }

    return result;
}