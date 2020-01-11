#define SHADOW_BIAS 0.001f
#define PI 3.14159265358979323846264338327950288

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
	float3		cCameraPos;
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

void CalcAttenuation(float3 P, float3 lightP, out float3 L, out float intensity)
{
	L = lightP - P;
	intensity = rcp(dot(L, L));

	L = L * sqrt(intensity);
}

// Schlick fresnel
float CalcFresnel(float Fo, float lDotH)
{
	return Fo + (1.f - Fo) * pow(1.f - lDotH, 5);
}

float CalcDGGX(float roughness, float nDotH)
{
	float roughnessSq = roughness * roughness;
	float denom = (1.f + nDotH * nDotH * (roughnessSq - 1.f));
	denom = PI * denom * denom;
	return roughnessSq / denom;
}

float CalcGeomGGX(float roughness, float cosTheta)
{
	float cosThetaSq = cosTheta * cosTheta;
	float numer = 2.f * cosTheta;
	float denom = cosTheta + sqrt(cosThetaSq + roughness * roughness * (1.f - cosThetaSq));
	return numer / denom;
}

float CalcGSmithGGX(float roughness, float nDotV, float nDotL)
{
	return CalcGeomGGX(roughness, nDotV) * CalcGeomGGX(roughness, nDotL);
}

float4 PSMain(PSInput input) : SV_TARGET
{
#ifdef HAS_NORMAL
	float3 result = float3(0.f, 0.f, 0.f);

	float3 L;
	float intensity;
	CalcAttenuation(input.Pw, cLightPosition, L, intensity);

	float3 V = normalize(cCameraPos - input.Pw);
	float3 H = normalize(V + L);

	float nDotL = saturate(dot(input.N, L));
	if (nDotL > 0.f)
	{
		if (intensity > 0.001f && dot(-L, cLightDirection.xyz) > cConeAngle)
		{
			float4 shadowMapCoord = mul(cWorldToShadowMap, float4(input.Pw, 1.f));
			shadowMapCoord.xyz /= shadowMapCoord.w;

			float shadow = tShadowMap.SampleCmp(sShadowSampler, shadowMapCoord.xy, shadowMapCoord.z - SHADOW_BIAS);

			float3 Kd = cLightColor.rgb * intensity * nDotL * shadow * (1.f / PI);

			L = normalize(-reflect(V, input.N));
			nDotL = saturate(dot(L, input.N));
			float lDotH = saturate(dot(L, H));
			float vDotN = saturate(dot(V, input.N));

			float roughness = 0.2f;
			float fresnel = CalcFresnel(0.9f, lDotH);
			float distribution = CalcDGGX(roughness, saturate(dot(input.N, H)));
			float geometry = CalcGSmithGGX(roughness, vDotN, nDotL);

			float radiance = (fresnel * distribution* geometry) / (4.f * nDotL * vDotN);

			result = radiance * shadow * intensity * nDotL * cLightColor.rgb;
			//result = fresnel.xxx / (4.f * nDotL * dot(input.N, V));
			result += Kd;
		}
	}

	return float4(result, 1.f);
#else
	return float4(input.uv.x, input.uv.y, 0, 1);
#endif
}
