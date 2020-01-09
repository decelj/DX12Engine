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

	float		cConeAngle;
	float3		cLightPosition;
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

float AttenuateLight(float3 pToLight)
{
	return rcp(dot(pToLight, pToLight));
}

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef HAS_NORMAL
	float3 result = float3(0.f, 0.f, 0.f);

	float lDotN = dot(input.N, -cLightDirection.xyz);
	if (lDotN > 0.f)
	{
		float3 fromLight = input.Pw - cLightPosition;
		float intensity = AttenuateLight(fromLight);

		fromLight = normalize(fromLight);
		if (intensity > 0.001f && dot(fromLight, cLightDirection.xyz) > cConeAngle)
		{
			float4 shadowMapCoord = mul(cWorldToShadowMap, float4(input.Pw, 1.f));
			shadowMapCoord.xyz /= shadowMapCoord.w;

			float shadow = tShadowMap.SampleCmp(sShadowSampler, shadowMapCoord.xy, shadowMapCoord.z - SHADOW_BIAS);
			result = lDotN * shadow * intensity * cLightColor.rgb;
		}
	}

	return float4(result, 1.f);
#else
	return float4(input.uv.x, input.uv.y, 0, 1);
#endif
}
