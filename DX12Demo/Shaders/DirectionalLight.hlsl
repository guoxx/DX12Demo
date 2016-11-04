#include "Inc/Common.hlsli"
#include "Inc/RSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=16), visibility=SHADER_VISIBILITY_PIXEL)" \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT, visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

HLSLConstantBuffer(DirectionalLightConstants, 0, g_Constants);

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

RootSigDeclaration
float4 PSMain(VSOutput In) : SV_TARGET
{
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_DepthTexture, g_Sampler, In.Texcoord, g_Constants.mInvView, g_Constants.mInvProj);

	float3 outRadiance = 0.0f;

	float3 shadowPos = mul(float4(gbuffer.Position, 1), g_Constants.m_DirLight.m_mViewProj).xyz;
	float2 shadowUV = float2((shadowPos.x + 1.0f) * 0.5f, (-shadowPos.y + 1.0f) * 0.5f);
	float occluderDepth = g_ShadowMap.Sample(g_Sampler, shadowUV);
	float shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;

	float3 L = -g_Constants.m_DirLight.m_Direction;
	float3 V = normalize(g_Constants.CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));
	float3 E = g_Constants.m_DirLight.m_Irradiance * NdotL * PI;

	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse) * E;
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L) * E;
	outRadiance += (diffuse + specular) * shadowMask;

	if (g_Constants.m_RSM.m_Enabled)
	{
		outRadiance += ShadeDirectionalLightRSM(gbuffer, g_Constants.m_RSM, g_Constants.m_DirLight,
			g_RSMIntensityTexture, g_RSMNormalTexture, g_ShadowMap, g_Sampler);
	}

	return float4(outRadiance, 1);
}