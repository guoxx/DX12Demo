#include "Inc/Common.hlsli"
#include "Inc/PointLight.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=16), visibility=SHADER_VISIBILITY_PIXEL)" \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT, visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSL_CB_DECL(DirectionalLight, Constants, 0,
{
	float4x4 mInvView;
	float4x4 mInvProj;
	float4 LightDirection;
	float4 LightIrradiance;
	float4 CameraPosition;
	float4x4 mLightViewProj;
	float4x4 mLightInvViewProj;

	int RSMEnabled;
	float RSMSampleRadius;
	float RSMSampleWeight;
	float RSMRadiusThreshold;
});

Texture2D<float4> g_GBuffer0 : register(t0);
Texture2D<float4> g_GBuffer1 : register(t1);
Texture2D<float4> g_GBuffer2 : register(t2);
Texture2D<float> g_DepthTexture : register(t3);
Texture2D<float> g_ShadowMap : register(t4);
Texture2D<float4> g_RSMIntensityTexture : register(t5);
Texture2D<float4> g_RSMNormalTexture : register(t6);

SamplerState g_Sampler : register(s0);

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD;
};

RootSigDeclaration
VSOutput VSMain(uint vertid : SV_VertexID)
{
	// 0 (0, 0)
	// 1 (1, 0)
	// 2 (0, 1)
	// 3 (1, 1)

	int x = vertid & 0x01;
	int y = (vertid >> 1) & 0x01;

	VSOutput Out;
	Out.Position = float4(x * 2 - 1, y * 2 - 1, 0, 1);
	Out.Texcoord = float2(x, 1 - y);

	return Out;
}

float3 ShadeDirectionalLightRSM(GBuffer gbuffer)
{
	float3 outRadiance = 0.0f;
	if (HLSL_CB_GET(0, RSMEnabled) != 0)
	{
		float3 shadowPos = mul(float4(gbuffer.Position, 1), HLSL_CB_GET(0, mLightViewProj)).xyz;
		float2 shadowUV = float2((shadowPos.x + 1.0f) * 0.5f, (-shadowPos.y + 1.0f) * 0.5f);

		for (uint i = 0; i < RSMSamplesCount; ++i)
		{
			float2 UVOffset = RSMSamplingPattern[i];
			UVOffset *= HLSL_CB_GET(0, RSMSampleRadius);

			float2 uv = shadowUV + UVOffset;

			// for direcional light, irradiance is stored in radiant intensity texture
			float3 VPLIrradiance = g_RSMIntensityTexture.SampleLevel(g_Sampler, uv, 0).xyz;

			float3 VPLNormal = g_RSMNormalTexture.SampleLevel(g_Sampler, uv, 0).xyz;
			VPLNormal = VPLNormal * 2.0f - 1.0f;

			float VPLDepthInLightSpace = g_ShadowMap.SampleLevel(g_Sampler, uv, 0);
			float3 ndcPosition = float3(uv.x * 2 - 1, -uv.y * 2 + 1, VPLDepthInLightSpace);
			float3 VPLPosition = mul(float4(ndcPosition, 1), HLSL_CB_GET(0, mLightInvViewProj)).xyz;

			// 1/PI already been applied in Diffuse_Lambert, and no distance falloff factor for directional light
			float3 VPLRadiance = VPLIrradiance;
			float3 E = VPLRadiance * PI;

			float3 L = normalize(VPLPosition - gbuffer.Position);
			float distance = length(VPLPosition - gbuffer.Position);
			float distanceEnd = RSMRadiusThreshold;
			float distanceFalloff = 0.0f;
			if (distance >= distanceEnd)
			{
				distanceFalloff = 0.0f;
			}
			else
			{
				distanceFalloff = (distanceEnd - distance) / distanceEnd;
			}
			E = E * saturate(dot(VPLNormal, -L)) * saturate(dot(gbuffer.Normal, L)) * distanceFalloff;
			outRadiance += E * gbuffer.Diffuse / PI;
		}
		outRadiance /= RSMSamplesCount;
		outRadiance *= RSMSampleWeight;
	}
	return outRadiance;
}

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_DepthTexture, g_Sampler, In.Texcoord, HLSL_CB_GET(0, mInvView), HLSL_CB_GET(0, mInvProj));

	float3 outRadiance = 0.0f;

	float3 shadowPos = mul(float4(gbuffer.Position, 1), HLSL_CB_GET(0, mLightViewProj)).xyz;
	float2 shadowUV = float2((shadowPos.x + 1.0f) * 0.5f, (-shadowPos.y + 1.0f) * 0.5f);
	float occluderDepth = g_ShadowMap.Sample(g_Sampler, shadowUV);
	float shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;

	float3 L = -HLSL_CB_GET(0, LightDirection).xyz;
	float3 V = normalize(HLSL_CB_GET(0, CameraPosition).xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));
	float3 E = HLSL_CB_GET(0, LightIrradiance).xyz * NdotL * PI;

	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse) * E;
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L) * E;
	outRadiance += (diffuse + specular) * shadowMask;

	outRadiance += ShadeDirectionalLightRSM(gbuffer);

	return float4(outRadiance, 1);
}