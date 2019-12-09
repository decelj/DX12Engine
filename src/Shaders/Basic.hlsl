struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer Constants : register(b0)
{
	float  cAspect;
	float2 cOffset;
	float  cPadding;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = position;
	result.position.xy *= cAspect;
	result.uv = (uv.xy + cOffset) * 0.5f;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(input.uv.x, input.uv.y, 0, 1);
}
