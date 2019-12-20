struct PSInput
{
	float4 position : SV_POSITION;
#ifdef HAS_NORMAL
	float3 normal : NORMAL;
#endif
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

//#define HAS_NORMAL

PSInput VSMain(
	float3 position : POSITION,
#ifdef HAS_NORMAL
	float3 normal : NORMAL,
#endif
	float4 uv : TEXCOORD)
{
	PSInput result;

	result.position = mul(cViewProj, float4(position, 1.f));
	result.uv = (uv.xy + cOffset) * 0.5f;
#ifdef HAS_NORMAL
	result.normal = normal;
#endif

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef HAS_NORMAL
	float3 lightDir = normalize(float3(1.f, 1.f, 1.f));
	float lDotN = dot(input.normal, lightDir);
	float lighting = saturate(lDotN) * 0.8f;
	return float4(lighting, lighting, lighting, 1.f);
	//return float4(input.normal * 0.5f + 0.5f, 1.f);
#else
	return float4(input.uv.x, input.uv.y, 0, 1);
#endif
}
