Texture2D ColorTexture : register(t0, space2);
SamplerState ColorSampler : register(s0, space2);

float4 main(float2 TexCoord : TEXCOORD0) : SV_Target0
{

	TexCoord *= 2;
	float4 color = ColorTexture.Sample(ColorSampler, TexCoord);

	return color;
}
