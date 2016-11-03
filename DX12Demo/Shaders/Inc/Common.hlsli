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

const static uint RSMSamplesCount = 64;
float2 RSMSamplingPattern[64] = {
float2(-0.3579344f, -0.3517149f),
float2(-0.3890468f, -0.7202561f),
float2(-0.2567423f, -0.0009548062f),
float2(-0.2508792f, -0.5212535f),
float2(-0.4917447f, -0.0206038f),
float2(-0.5943144f, -0.310339f),
float2(-0.08068617f, -0.2073122f),
float2(-0.5964261f, -0.6146008f),
float2(-0.06257999f, -0.4119583f),
float2(-0.003197781f, -0.6396211f),
float2(-0.1764597f, -0.8859963f),
float2(0.2531657f, -0.4200461f),
float2(0.05629459f, -0.9949071f),
float2(0.166944f, -0.7750345f),
float2(0.3332334f, -0.6005474f),
float2(0.2902015f, -0.9262927f),
float2(0.472591f, -0.8016518f),
float2(-0.7654607f, -0.4435649f),
float2(-0.5578633f, -0.8274426f),
float2(-0.5786387f, 0.1987577f),
float2(0.07728124f, 0.04528601f),
float2(-0.2269627f, 0.2174418f),
float2(-0.8871316f, -0.1302793f),
float2(-0.7454758f, -0.0006463315f),
float2(-0.9457548f, -0.3118609f),
float2(-0.001683236f, 0.2770302f),
float2(0.2118009f, -0.1378248f),
float2(0.509697f, -0.02626915f),
float2(0.3068107f, 0.1078179f),
float2(0.4372152f, -0.2751352f),
float2(0.2258023f, 0.3498218f),
float2(0.7067862f, -0.3505104f),
float2(0.4158016f, 0.2694948f),
float2(0.6265055f, 0.1402356f),
float2(0.8327943f, 0.02783612f),
float2(0.6864228f, -0.1073536f),
float2(0.6737096f, -0.6840767f),
float2(0.490763f, -0.4835717f),
float2(-0.08402723f, 0.4619588f),
float2(0.3806773f, 0.6527942f),
float2(0.1396271f, 0.5422848f),
float2(0.5649002f, 0.4364094f),
float2(0.8278279f, -0.5233458f),
float2(0.9161091f, -0.3291859f),
float2(0.7785613f, 0.3681582f),
float2(0.1783977f, 0.8567262f),
float2(0.5152924f, 0.8373909f),
float2(0.7113628f, 0.6190548f),
float2(0.01140799f, 0.7131786f),
float2(-0.9841616f, 0.1477079f),
float2(-0.7976238f, 0.2312148f),
float2(-0.693472f, 0.406074f),
float2(-0.3649378f, 0.4050539f),
float2(-0.5691569f, 0.5673698f),
float2(-0.5047651f, 0.7472093f),
float2(-0.2069313f, 0.7437413f),
float2(0.9562784f, 0.2554863f),
float2(0.9646869f, -0.1418799f),
float2(-0.4015996f, 0.9155728f),
float2(-0.8067811f, 0.5761073f),
float2(0.0941247f, -0.290804f),
float2(-0.05625709f, 0.9482339f),
float2(-0.3657628f, 0.6047338f),
float2(-0.3679701f, -0.9156333f),
};
