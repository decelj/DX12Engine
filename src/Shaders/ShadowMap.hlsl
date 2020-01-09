struct VSOutput
{
	float4 position : SV_POSITION;
};

cbuffer Constants : register(b0)
{
	float4x4 cViewProj;
};

cbuffer GeometryConstants : register(b1)
{
	float4x4 cModel;
};

VSOutput VSMain(
	float3 position : POSITION,
#ifdef HAS_NORMAL
	float3 normal : NORMAL,
#endif
	float4 uv : TEXCOORD)
{
	VSOutput result;

	float4x4 modelViewProj = mul(cViewProj, cModel);

	result.position = mul(modelViewProj, float4(position, 1.f));
	return result;
}
