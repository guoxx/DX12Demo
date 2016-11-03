#include "HLSLShared.h"
#include "BRDF.hlsli"


#define RootSigBegin \
[RootSignature("RootFlags(0)"

#define RootSigEnd \
)]


// G-Bufer Layout
/*

+----------------+----------------+----------------+----------------+
| diffuse                                          +                +
+----------------+----------------+----------------+----------------+
| specular                                         +                +
+----------------+----------------+----------------+----------------+
| normal                                           + roughness      +
+----------------+----------------+----------------+----------------+

*/

struct GBufferOutput
{
	float4 Diffuse          : SV_TARGET0;
	float4 Specular         : SV_TARGET1;
	float4 Normal_Roughness : SV_TARGET2;
};

struct GBuffer
{
	float3 Diffuse;
	float3 Specular;
	float3 Normal;
	float Roughness;


	// --
	float Depth;
	float3 Position;
};

struct RSMOutput
{
	float4 Intensity	: SV_TARGET0;
	float4 Normal		: SV_TARGET1;
};

struct RSMBuffer
{
	float3 Intensity;
	float3 Normal;

	// --
	float Depth;
	float3 Position;
};


GBufferOutput GBufferEncode(GBuffer gbuffer)
{
	GBufferOutput Out;

	Out.Diffuse = float4(gbuffer.Diffuse, 1.0f);
	Out.Specular = float4(gbuffer.Specular, 1.0f);
	Out.Normal_Roughness.xyz = (gbuffer.Normal + 1.0f) * 0.5f;
	Out.Normal_Roughness.w = gbuffer.Roughness;

	return Out;
}

GBuffer GBufferDecode(Texture2D<float4> RT0, Texture2D<float4> RT1, Texture2D<float4> RT2, Texture2D<float> DepthBuffer, SamplerState samp, float2 UV,
	float4x4 mInvView, float4x4 mInvProj)
{
	GBuffer gbuffer;

	float4 Diffuse = RT0.SampleLevel(samp, UV, 0);
	float4 Specular = RT1.SampleLevel(samp, UV, 0);
	float4 Normal_Roughness = RT2.SampleLevel(samp, UV, 0);
	gbuffer.Diffuse = Diffuse.xyz;
	gbuffer.Specular = Specular.xyz;
	gbuffer.Normal = Normal_Roughness.xyz * 2.0f - 1.0f;
	gbuffer.Roughness = Normal_Roughness.w;

	gbuffer.Depth = DepthBuffer.SampleLevel(samp, UV, 0);

	// Assume DX style texture coordinate
	float3 ndcPos = float3(UV.x * 2.0f - 1.0f, (1.0f - UV.y) * 2.0f - 1.0f, gbuffer.Depth);
	float4 csPos = mul(float4(ndcPos, 1), mInvProj);
	csPos /= csPos.w;
	// world space position
	gbuffer.Position = mul(csPos, mInvView).xyz;

	return gbuffer;
}

RSMOutput RSMBufferEncode(RSMBuffer rsmbuffer)
{
	RSMOutput Out;
	Out.Intensity = float4(rsmbuffer.Intensity, 1.0f);
	Out.Normal = float4((rsmbuffer.Normal + 1.0f) * 0.5f, 1.0f);
	return Out;
}

RSMBuffer RSMBufferDecode(Texture2D<float4> RT0, Texture2D<float4> RT1, Texture2D<float> DepthBuffer, SamplerState samp, float2 UV,
	float4x4 mInvView, float4x4 mInvProj)
{
	RSMBuffer rsmbuffer;

	float4 Intensity = RT0.SampleLevel(samp, UV, 0);
	float4 Normal= RT1.SampleLevel(samp, UV, 0);
	rsmbuffer.Intensity = Intensity.xyz;
	rsmbuffer.Normal = Normal.xyz * 2.0f - 1.0f;

	rsmbuffer.Depth = DepthBuffer.SampleLevel(samp, UV, 0);

	// Assume DX style texture coordinate
	float3 ndcPos = float3(UV.x * 2.0f - 1.0f, (1.0f - UV.y) * 2.0f - 1.0f, rsmbuffer.Depth);
	float4 csPos = mul(float4(ndcPos, 1), mInvProj);
	csPos /= csPos.w;
	// world space position
	rsmbuffer.Position = mul(csPos, mInvView).xyz;

	return rsmbuffer;
}

// index of refraction to F0
float IorToF0_Dielectric(float ior)
{
	return pow(ior - 1, 2) / pow(ior + 1, 2);
}
