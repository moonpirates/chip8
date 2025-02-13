cbuffer UniformBlock : register(b0, space3)
{
    uint canvasWidth;
    uint canvasHeight;
};

float4 main(float4 Color : TEXCOORD0) : SV_Target0
{
    return Color;
}
