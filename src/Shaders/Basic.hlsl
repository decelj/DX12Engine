#define SHADOW_BIAS 0.001f

struct PSInput
{
	float4 P : SV_POSITION;
	float3 Pw : POSITION;
#ifdef HAS_NORMAL
	float3 N : NORMAL;
#endif
	float2 uv : TEXCOORD;
};

cbuffer Constants : register(b0)
{
	float4x4	cView;
	float4x4	cProj;
	float4x4	cViewProj;
	float		cAspect;
	float2		cOffset;
	float		cPadding;
};

cbuffer GeometryConstants : register(b1)
{
	float4x4	cModel;
}

cbuffer LightConstants : register(b2)
{
	float4x4	cWorldToShadowMap;
	float4		cLightDirection;
	float4		cLightColor;
}

SamplerComparisonState sShadowSampler : register(s0);
SamplerState sPointSampler : register(s1);
Texture2D tShadowMap : register(t0);

PSInput VSMain(
	float3 position : POSITION,
#ifdef HAS_NORMAL
	float3 normal : NORMAL,
#endif
	float4 uv : TEXCOORD)
{
	PSInput result;

	float4x4 modelViewProj = mul(cViewProj, cModel);

	result.Pw = mul(cModel, float4(position, 1.f)).xyz;
	result.P = mul(modelViewProj, float4(position, 1.f));
	result.uv = uv.xy;
#ifdef HAS_NORMAL
	result.N = mul(normal, (float3x3)cModel);
#endif

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef HAS_NORMAL
	float4 shadowMapCoord = mul(cWorldToShadowMap, float4(input.Pw, 1.f));
	shadowMapCoord.xyz /= shadowMapCoord.w;

	float shadow = tShadowMap.SampleCmp(sShadowSampler, shadowMapCoord.xy, shadowMapCoord.z - SHADOW_BIAS);
	float lDotN = dot(input.N, -cLightDirection.xyz);
	float3 lighting = saturate(lDotN) * shadow * cLightColor.rgb;
	return float4(lighting, 1.f);
	//return float4(input.normal * 0.5f + 0.5f, 1.f);
#else
	return float4(input.uv.x, input.uv.y, 0, 1);
#endif
}
