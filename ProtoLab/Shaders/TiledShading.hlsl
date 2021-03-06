#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"
#include "Inc/PointLight.hlsli"
#include "Inc/RSM.hlsli"
#include "Inc/VSM.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0) " \
", SRV(t0) " \
", SRV(t1) " \
", SRV(t2) " \
", DescriptorTable(SRV(t3, numDescriptors=6), UAV(u0))" \
", DescriptorTable(SRV(t16, numDescriptors=unbounded))" \
RootSigEnd

HLSLConstantBuffer(TiledShadingConstants, 0, g_Constants);

StructuredBuffer<DirectionalLight> g_DirectionalLights : register(t0);
StructuredBuffer<PointLight> g_PointLights : register(t1);
StructuredBuffer<LightNode> g_LightNodes: register(t2);

Texture2D<float4> g_GBuffer0 : register(t3);
Texture2D<float4> g_GBuffer1 : register(t4);
Texture2D<float4> g_GBuffer2 : register(t5);
Texture2D<float4> g_GBuffer3 : register(t6);
Texture2D<float> g_DepthTexture : register(t7);
Texture2D<float> g_ScreenSpaceShadowMap : register(t8);

Texture2D g_ShadowMaps[] : register(t16);

RWTexture2D<float4> g_LightingSurface : register(u0);

groupshared uint gs_NumLightsPerTile;
groupshared uint gs_LightIdxPerTile[MAX_LIGHT_NODES_PER_TILE];

float3 ShadeDirectionalLight(float2 uv, GBuffer gbuffer, DirectionalLight directionalLight)
{
	float3 outRadiance = 0.0f;

	float shadowMask = g_ScreenSpaceShadowMap.SampleLevel(g_StaticPointClampSampler, uv, 0);

	float3 L = -directionalLight.m_Direction.xyz;
	float3 V = normalize(g_Constants.m_CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));
	float3 E = directionalLight.m_Irradiance.xyz;

	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse);
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L);
	outRadiance = (diffuse + specular) * shadowMask * E * NdotL;

	if (g_Constants.m_RSM.m_Enabled)
	{
		outRadiance += ShadeDirectionalLightRSM(gbuffer,
			g_Constants.m_RSM,
			directionalLight,
			g_ShadowMaps[directionalLight.m_RSMIntensityTexId],
			g_ShadowMaps[directionalLight.m_RSMNormalTexId],
			g_ShadowMaps[directionalLight.m_ShadowMapTexId],
			g_StaticPointClampSampler);
	}

	return outRadiance;
}

float3 ShadePointLight(GBuffer gbuffer, PointLight pointLight)
{
	float shadowMask = 1.0f;
	if (pointLight.m_FirstShadowMapTexId != -1)
	{
		int face = GetFaceOfPointLightShadowMap(pointLight.m_PositionAndRadius.xyz, gbuffer.Position);
		float4 shadowPos = mul(float4(gbuffer.Position, 1), pointLight.m_mViewProj[face]);
		shadowPos /= shadowPos.w;
		float occluderDepth = g_ShadowMaps[pointLight.m_FirstShadowMapTexId + face].SampleLevel(g_StaticPointClampSampler, shadowPos.xy * float2(1, -1) * 0.5 + 0.5, 0);
		shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;
	}

	float3 L = normalize(pointLight.m_PositionAndRadius.xyz - gbuffer.Position);
	float3 V = normalize(g_Constants.m_CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));

	float dist = length(pointLight.m_PositionAndRadius.xyz - gbuffer.Position);
	float radius = pointLight.m_PositionAndRadius.w;
	float3 E = PointLightIrradiance(pointLight.m_RadiantPower.xyz, dist, radius);
	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse);
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L);
    float3 bsdf = diffuse + specular;
    return bsdf * E * NdotL * shadowMask;
}

[numthreads(LIGHT_CULLING_NUM_THREADS_XY, LIGHT_CULLING_NUM_THREADS_XY, 1)]
RootSigDeclaration
void CSMain(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	uint2 tileId = Gid.xy;
	uint linearTileId = LinearizeTileId(tileId, g_Constants.m_NumTileX, g_Constants.m_NumTileY);
	uint linearThreadId = LinearizeThreadId(GTid.xy);

	if (linearThreadId == 0)
	{
		gs_NumLightsPerTile = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	uint startOffset = linearTileId * MAX_LIGHT_NODES_PER_TILE;

	// TODO: HACK
	if (linearThreadId == 0)
	{
		for (uint i = 0; i < MAX_LIGHT_NODES_PER_TILE; ++i)
		{
			uint offset = startOffset + i;
			if (g_LightNodes[offset].m_LightIndex == LIGHT_NODE_INVALID)
			{
				break;
			}

			gs_LightIdxPerTile[i] = g_LightNodes[offset].m_LightIndex;
			gs_NumLightsPerTile += 1;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	float2 texcoord = float2(DTid.x + 0.5f, DTid.y + 0.5f);
	float2 uv = float2(texcoord.x / g_Constants.m_ScreenWidth, texcoord.y / g_Constants.m_ScreenHeight);
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_GBuffer3, g_DepthTexture, g_StaticPointClampSampler, uv, g_Constants.m_mInvView, g_Constants.m_mInvProj);

	float3 outRadiance = 0.0f;

	for (uint visDirectionalLightIdx = 0; visDirectionalLightIdx < g_Constants.m_NumDirectionalLights; ++visDirectionalLightIdx)
	{
		outRadiance += ShadeDirectionalLight(uv, gbuffer, g_DirectionalLights[visDirectionalLightIdx]);
	}

	for (uint visPointLightIdx = 0; visPointLightIdx < gs_NumLightsPerTile; ++visPointLightIdx)
	{
		outRadiance += ShadePointLight(gbuffer, g_PointLights[gs_LightIdxPerTile[visPointLightIdx]]);
	}

	g_LightingSurface[DTid.xy] = float4(outRadiance, 1);
}