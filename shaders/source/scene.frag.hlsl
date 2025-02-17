cbuffer UniformBlock : register(b0, space3)
{
	uint canvasWidth;
	uint canvasHeight;
};

struct Input
{
	float4 Color : TEXCOORD0;
	float4 Position : SV_Position;
};

float4 main(Input Input) : SV_Target0
{
	return Input.Color;
}

