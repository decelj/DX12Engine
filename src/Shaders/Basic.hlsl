struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer Constants : register(b0)
{
	float4x4 cView;
	float4x4 cProj;
	float4x4 cViewProj;
	float  cAspect;
	float2 cOffset;
	float  cPadding;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = mul(cViewProj, position);
	result.uv = (uv.xy + cOffset) * 0.5f;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
	return float4(input.uv.x, input.uv.y, 0, 1);
}
