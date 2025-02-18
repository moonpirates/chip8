#define PI 3.14159265359
#define FG_COLOR float4(0.196, 1.0, 0.4, 1.0)
#define BG_COLOR float4(0.2, 0.2, 0.2, 1.0)
#define MARGIN float2(0.05, 0.05)
#define CURVATURE 5.0
#define BLUR_SIZE float2(2.0, 2.0)
#define BLOOM_WEIGHT 0.75
#define BLOOM_SIZE float2(10.0, 10.0)
#define SCANLINE_FREQUENCY 0.18
#define SUB_PIXEL_STRENGTH 0.65
#define LEVELS_CURVE_STRENGTH 1.7
#define VIGNETTE_MAG 0.4 
#define VIGNETTE_MULTIPLIER 1.5

cbuffer UniformBlock : register(b0, space3)
{
	float2 windowSize;
};

Texture2D ColorTexture : register(t0, space2);
SamplerState ColorSampler : register(s0, space2);

float3 sampleTexture(float2 uv, Texture2D ColorTexture, SamplerState ColorSampler)
{
	return lerp(BG_COLOR, FG_COLOR, ColorTexture.Sample(ColorSampler, uv).r).rgb;
}

float3 boxBlur(float2 uv, float2 texelSize, float2 blurSize, Texture2D ColorTexture, SamplerState ColorSampler)
{
	const float2 TEXEL_BLUR_SIZE = blurSize * texelSize;
	const float2 ORIGINAL_UV = uv - TEXEL_BLUR_SIZE;
	float numSamples = 0.0;
	float3 returnColor = float3(0.0, 0.0, 0.0);

	for (uv.y = ORIGINAL_UV.y; uv.y < ORIGINAL_UV.y + TEXEL_BLUR_SIZE.y * 2.0; uv.y += texelSize.y)
	{
		if (uv.y < 0.0)
			continue;

		if (uv.y >= 1.0)
			break;

		for (uv.x = ORIGINAL_UV.x; uv.x < ORIGINAL_UV.x + TEXEL_BLUR_SIZE.x * 2.0; uv.x += texelSize.x)
		{
			if (uv.x < 0.0)
				continue;

			if (uv.x >= 1.0)
				break;

			returnColor += sampleTexture(uv, ColorTexture, ColorSampler);
			numSamples++;
		}
	}

	return returnColor / numSamples;
}

float4 main(float2 uv : TEXCOORD0) : SV_Target0
{
	const float2 ORIGINAL_UV = uv;
	const float2 TEXEL_SIZE = 1.0 / windowSize;
	const int2 PX_POS = uv * windowSize;

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
				
		float3 originalValue = boxBlur(uv, TEXEL_SIZE, BLUR_SIZE, ColorTexture, ColorSampler);
		float3 bloomValue = boxBlur(uv, TEXEL_SIZE, BLOOM_SIZE, ColorTexture, ColorSampler) * BLOOM_WEIGHT;
		float3 mix = max(originalValue, bloomValue);

		color = float4(mix.r, mix.g, mix.b, 1.0);
	}

	// Scanlines
	float scanline = cos(PX_POS.y * PI * 2.0 * SCANLINE_FREQUENCY);
	scanline = (scanline + 1.0) / 2.0; // move from [-1..1] to [0..1]
	scanline *= 2.0; // Oversaturate so we get more whites in the scan lines
	scanline = saturate(scanline); // clamp
	scanline = (scanline / 4) + 0.75; // move to [0.75..1.0]
	color *= scanline;

	// Sub pixels
	float xMod = PX_POS.x % 3;
	const float RGB_FACTOR = 1.0 - SUB_PIXEL_STRENGTH;
	if (xMod == 0)
	{
		color.g *= RGB_FACTOR;
		color.b *= RGB_FACTOR;
	}
	else if (xMod == 1)
	{
		color.r *= RGB_FACTOR;
		color.b *= RGB_FACTOR;
	}
	else if (xMod == 2)
	{
		color.r *= RGB_FACTOR;
		color.g *= RGB_FACTOR;
	}

	// Apply curve to levels
	color = 1.0 - pow(abs(color - 1.0), LEVELS_CURVE_STRENGTH);

	// Vignette
	float2 vignetteUV = ORIGINAL_UV;
	vignetteUV *= 1.0 - vignetteUV.yx;
	float vignette = vignetteUV.x * vignetteUV.y * 15.0;
	vignette = pow(vignette, VIGNETTE_MAG) * VIGNETTE_MULTIPLIER;
	color *= vignette;

	return color;
}
