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


/*float4 main(float4 position : SV_Position) : SV_Target0
{
	float2 pos = float2(position.x / canvasWidth, position.y / canvasHeight);
	return float4(pos.x, pos.y, 0., 1.);
}*/

//https://www.shadertoy.com/view/WsVSzV
//https://www.youtube.com/watch?v=aWdySZ0BtJs

float4 main(Input Input) : SV_Target0
{
	//return float4(uv.x, uv.y, 0, 1);
	//Input.Color = float4(1.0, 1.0, 1.0, 1.0);

	float2 uv = float2(Input.Position.x / canvasWidth, 1. - Input.Position.y / canvasHeight);
	float2 originalUV = uv;
	uv = uv * 2.0 - 1.0;

	const float CURVATURE = 0.15;
	float2 offset = uv.yx * CURVATURE;
	uv = uv + uv * offset * offset;
	uv = uv * 0.5 + 0.5;

	if (uv.x < 0.0 || uv.x >= 1.0 || uv.y < 0.0 || uv.y >= 1.0)
		return float4(0, 0, 0, 1);
	

	//float horizontalScanlineMultiplier = (sin(Input.Position.y) + 5.0) / 6.0;
	//Input.Color *= horizontalScanlineMultiplier;

	//////////vignette = saturate(vignette);
	float vignette = pow(uv.x * (1.0 - uv.x) * uv.y * (1.0 - uv.y), 0.25) * 2.5;
	Input.Color *= vignette;

	const float CHANNEL_DARKEN_FACTOR = 0.6;
	int x = Input.Position.x;
	int y = Input.Position.y;

	if (x % 3 == 0)
	{
		Input.Color.g *= CHANNEL_DARKEN_FACTOR;
		Input.Color.b *= CHANNEL_DARKEN_FACTOR;
	}
	else if (x % 3 == 1)
	{
		Input.Color.r *= CHANNEL_DARKEN_FACTOR;
		Input.Color.b *= CHANNEL_DARKEN_FACTOR;
	}
	else if (x % 3 == 2)
	{
		Input.Color.r *= CHANNEL_DARKEN_FACTOR;
		Input.Color.g *= CHANNEL_DARKEN_FACTOR;
	}

	const int LINE_HEIGHT = 6;

	if (x % 6 >= 3 && x % 6 <= 5)
		y += LINE_HEIGHT / 2;

	if (y % LINE_HEIGHT == LINE_HEIGHT - 1)
		Input.Color *= CHANNEL_DARKEN_FACTOR;

	return Input.Color;
}

