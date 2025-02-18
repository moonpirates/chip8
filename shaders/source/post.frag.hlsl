#define PI 3.14159265359
#define FG_COLOR float4(0.196, 1.0, 0.4, 1.0)
#define BG_COLOR float4(0.2, 0.2, 0.2, 1.0)
#define MARGIN float2(0.05, 0.05)
#define CURVATURE 5.0

cbuffer UniformBlock : register(b0, space3)
{
	float2 windowSize;
};

Texture2D ColorTexture : register(t0, space2);
SamplerState ColorSampler : register(s0, space2);



// float3 bloom(float2 uv)
// {
	
// }
//#define BLUR_SIZE 10.0
//#define BLUR_QUALITY 4.0

float luminance(float3 rgb)
{
	const float3 W = float3(0.2125, 0.7154, 0.0721);
	return dot(rgb, W);
}

// float sampleValue(float2 uv, Texture2D ColorTexture, SamplerState ColorSampler)
// {
// 	return ColorTexture.Sample(ColorSampler, uv).r;
// }

float3 sampleTexture(float2 uv, Texture2D ColorTexture, SamplerState ColorSampler)
{
	return lerp(BG_COLOR, FG_COLOR, ColorTexture.Sample(ColorSampler, uv).r);
}

float3 boxBlur(float2 uv, float2 texelSize, float2 blurSize, Texture2D ColorTexture, SamplerState ColorSampler)
{
	const float2 TEXEL_BLUR_SIZE = blurSize * texelSize;
	const float2 originalUV = uv - TEXEL_BLUR_SIZE;
	float numSamples = 0.0;
	float3 returnColor = float3(0.0, 0.0, 0.0);

	for (uv.y = originalUV.y; uv.y < originalUV.y + TEXEL_BLUR_SIZE.y * 2.0; uv.y += texelSize.y) // use quality for step size
	{
		if (uv.y < 0.0)
			continue;

		if (uv.y >= 1.0)
			break;

		for (uv.x = originalUV.x; uv.x < originalUV.x + TEXEL_BLUR_SIZE.x * 2.0; uv.x += texelSize.x) // use quality for step size
		{
			if (uv.x < 0.0)
				continue;

			if (uv.x >= 1.0)
				break;

			returnColor += sampleTexture(uv, ColorTexture, ColorSampler);// distance from center laten aflopen? fake gaus?
			numSamples++;
		}
	}

	return returnColor / numSamples;
}

float smoothSquareWave(float t, float freq, float smoothness) 
{
	return tanh(smoothness * sin(2.0 * PI * freq * t));
}

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	const float2 ORIGINAL_UV = uv;
	const float2 TEXEL_SIZE = 1.0 / windowSize;
	const int2 pxPos = uv * windowSize;

	// Curvature
	uv = uv * 2.0 - 1.0;
	float2 offset = uv.yx / CURVATURE;
	uv = uv + uv * offset * offset;
	uv = uv * 0.5 + 0.5;

	if (uv.x < 0.0 || uv.x >= 1.0 || uv.y < 0.0 || uv.y >= 1.0)
		return float4(0.0, 0.0, 0.0, 1.0);

	// Sample CHIP8
	float4 color;
	if (uv.x < MARGIN.x || uv.x > 1.0 - MARGIN.x || uv.y < MARGIN.y || uv.y > 1 - MARGIN.y)
	{
		color = BG_COLOR;
	}
	else
	{
		uv.x = (uv.x - MARGIN.x) / (1.0 - MARGIN.x * 2.0);
		uv.y = (uv.y - MARGIN.y) / (1.0 - MARGIN.y * 2.0);
		
		const float2 BLUR_SIZE = float2(2.0, 2.0);
		const float BLOOM_WEIGHT = 0.75;
		const float2 BLOOM_SIZE = float2(15.0, 10.0);
		
		float3 originalValue = boxBlur(uv, TEXEL_SIZE, BLUR_SIZE, ColorTexture, ColorSampler);
		float3 bloomValue = boxBlur(uv, TEXEL_SIZE, BLOOM_SIZE, ColorTexture, ColorSampler) * BLOOM_WEIGHT;
		float3 mix = max(originalValue, bloomValue);

		color = float4(mix.r, mix.g, mix.b, 1.0);
	}

	//color = float4(1,1,1,1);

	if (true)
	{
		// Scanlines
		float freq = 0.18;
		float scanline = cos(pxPos.y * PI * 2.0 * freq);
		scanline = (scanline + 1.0) / 2.0; // move from [-1..1] to [0..1]
		scanline *= 2.0; // Oversaturate so we get more whites in the scanlines
		scanline = saturate(scanline);
		scanline /= 4;
		scanline += 0.75;
		color *= scanline;

		float mod = pxPos.x % 3;
		const float RGB_FACTOR = 0.65;

		if (mod == 0)
		{
			color.g *= RGB_FACTOR;
			color.b *= RGB_FACTOR;
		}
		else if (mod == 1)
		{
			color.r *= RGB_FACTOR;
			color.b *= RGB_FACTOR;
		}
		else if (mod == 2)
		{
			color.r *= RGB_FACTOR;
			color.g *= RGB_FACTOR;
		}
	}

	const float CURVE_STRENGTH = 1.7;
	color = 1.0 - pow(abs(color - 1.0), CURVE_STRENGTH);

	// Vignette
	float vignette = pow(ORIGINAL_UV.x * (1.0 - ORIGINAL_UV.x) * ORIGINAL_UV.y * (1.0 - ORIGINAL_UV.y), 0.25) * 2.5;
	color *= vignette;





	//color = float4(windowSize.x / 1000.0, windowSize.y / 1000.0, 0.0, 1.0);



	return color;
}
