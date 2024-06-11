#ifndef FULLSCREEN_TRIANGLE_HLSL
#define FULLSCREEN_TRIANGLE_HLSL

struct VS_OUTPUT
{
    float4 vPosition  : SV_POSITION;
    float2 vUv        : TEXCOORD0;
};

static const float4 cvPositions[3] =
{
    { -1.0,  3.0, 0.0, 1.0 },
    {  3.0, -1.0, 0.0, 1.0 },
    { -1.0, -1.0, 0.0, 1.0 }
};

VS_OUTPUT main(in uint uVertexID : SV_VertexID)
{
    VS_OUTPUT output;
    output.vPosition = cvPositions[uVertexID];

    // Correct UV calculation
    output.vUv.x = (cvPositions[uVertexID].x + 1.0) * 0.5;
    output.vUv.y = (1.0 - cvPositions[uVertexID].y) * 0.5;

    return output;
};

#endif // FULLSCREEN_TRIANGLE_HLSL
