#include "Inc/Common.hlsli"
#include "Inc/PointLight.hlsli"
#include "Inc/GBuffer.hlsli"
#include "Inc/BRDF.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=11), visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

struct Constants
{
	float4x4 mInvView;
	float4x4 mInvProj;
	float4 CameraPosition;

	float4 LightPositionAndRadius;
	float4 LightRadiantPower;
	float4x4 mLightViewProj[6];
};
HLSLConstantBuffer(Constants, 0, g_Constants);

Texture2D<float4> g_GBuffer0 : register(t0);
Texture2D<float4> g_GBuffer1 : register(t1);
Texture2D<float4> g_GBuffer2 : register(t2);
Texture2D<float4> g_GBuffer3 : register(t3);
Texture2D<float> g_DepthTexture : register(t4);
Texture2D<float> g_PointLightShadowMap[6] : register(t5);


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

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_GBuffer3, g_DepthTexture, g_StaticPointClampSampler, In.Texcoord, g_Constants.mInvView, g_Constants.mInvProj);

	float3 outRadiance = 0.0f;

	int face = GetFaceOfPointLightShadowMap(g_Constants.LightPositionAndRadius.xyz, gbuffer.Position);
	float4 shadowPos = mul(float4(gbuffer.Position, 1), g_Constants.mLightViewProj[face]);
	shadowPos /= shadowPos.w;
	float occluderDepth = g_PointLightShadowMap[face].Sample(g_StaticPointClampSampler, shadowPos.xy * float2(1, -1) * 0.5 + 0.5);
	float shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;

	float3 L = normalize(g_Constants.LightPositionAndRadius.xyz - gbuffer.Position);
	float3 V = normalize(g_Constants.CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));

	float dist = length(g_Constants.LightPositionAndRadius.xyz - gbuffer.Position);
	float radius = g_Constants.LightPositionAndRadius.w;
	float3 E = PointLightIrradiance(g_Constants.LightRadiantPower.xyz, dist, radius);
	//if (any(E))
	{
		float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse);
		float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L);
        float3 bsdf = diffuse + specular;
		outRadiance += bsdf * E * NdotL * shadowMask;
	}

	return float4(outRadiance, 1);
}