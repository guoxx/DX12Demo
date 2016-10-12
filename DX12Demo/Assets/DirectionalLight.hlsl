#include "Common.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0, visibility = SHADER_VISIBILITY_ALL)" \
", DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", DescriptorTable(SRV(t2, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", DescriptorTable(SRV(t3, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)" \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT, visibility=SHADER_VISIBILITY_PIXEL)" \
RootSigEnd

struct Constants
{
	float4x4 mInvView;
	float4x4 mInvProj;
	float4 LightDirection;
	float4 LightIrradiance;
	float4 CameraPosition;
};

Texture2D<float4> g_GBuffer0 : register(t0);
Texture2D<float4> g_GBuffer1 : register(t1);
Texture2D<float4> g_GBuffer2 : register(t2);
Texture2D<float> g_DepthTexture : register(t3);

SamplerState g_Sampler : register(s0);
ConstantBuffer<Constants> g_Constants : register(b0);

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

	float3 L = -g_Constants.LightDirection;
	float3 V = normalize(g_Constants.CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));
	float3 E = g_Constants.LightIrradiance * NdotL * PI;

	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse) * E;
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L) * E;
	return float4(diffuse + specular, 1);
}