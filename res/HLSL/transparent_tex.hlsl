struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0)
{
    float2 TextureSize;
};

float4 main(PSInput input) : SV_TARGET
{

    float2 adjustedUv = float2(input.TexCoord.x, input.TexCoord.y) * TextureSize;

    float2 gridPos = floor(adjustedUv / 8.0);

    float checker = fmod(gridPos.x + gridPos.y, 2.0);

    return checker == 0 ? float4(0.8, 0.8, 0.8, 1) : float4(1, 1, 1, 1);
}
