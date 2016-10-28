#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"
#include "Inc/PointLight.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0) " \
", SRV(t0) " \
", SRV(t1) " \
", DescriptorTable(SRV(t2, numDescriptors=4), UAV(u0))" \
", StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_POINT)" \
RootSigEnd

ConstantBuffer<TiledShadingConstants> g_Constants : register(b0);

StructuredBuffer<PointLight> g_PointLights : register(t0);
StructuredBuffer<LightNode> g_LightNodes: register(t1);

Texture2D<float4> g_GBuffer0 : register(t2);
Texture2D<float4> g_GBuffer1 : register(t3);
Texture2D<float4> g_GBuffer2 : register(t4);
Texture2D<float> g_DepthTexture : register(t5);

SamplerState g_PointSampler : register(s0);

RWTexture2D<float4> g_LightingSurface : register(u0);

groupshared uint gs_NumLightsPerTile;
groupshared uint gs_LightIdxPerTile[MAX_LIGHT_NODES_PER_TILE];

float3 ShadePointLight(GBuffer gbuffer, PointLight pointLight)
{
	/*
	int face = GetFaceOfPointLightShadowMap(pointLight.m_Position.xyz, gbuffer.Position);
	float4 shadowPos = mul(float4(gbuffer.Position, 1), pointLight.m_mViewProj[face]);
	shadowPos /= shadowPos.w;
	float occluderDepth = g_PointLightShadowMap[face].Sample(g_PointSampler, shadowPos.xy * float2(1, -1) * 0.5 + 0.5);
	// TODO: shouldn't hard code depth bias
	occluderDepth += 0.00001;
	float shadowMask = occluderDepth < shadowPos.z ? 0.0f : 1.0f;
	*/
	float shadowMask = 1.0f;

	float3 L = normalize(pointLight.m_Position.xyz - gbuffer.Position);
	float3 V = normalize(g_Constants.m_CameraPosition.xyz - gbuffer.Position);
	float3 N = gbuffer.Normal;
	float NdotL = saturate(dot(N, L));

	float dist = length(pointLight.m_Position.xyz - gbuffer.Position);
	float radiusStart = pointLight.m_RadiusParam.x;
	float radiusEnd = pointLight.m_RadiusParam.y;
	float3 E = PointLightIrradiance(pointLight.m_Intensity.xyz, dist, radiusStart, radiusEnd) * NdotL * PI;
	float3 diffuse = Diffuse_Lambert(gbuffer.Diffuse) * E;
	float3 specular = MicrofacetSpecular(gbuffer.Specular, gbuffer.Roughness, V, N, L) * E;
	return (diffuse + specular) * shadowMask;
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
	GBuffer gbuffer = GBufferDecode(g_GBuffer0, g_GBuffer1, g_GBuffer2, g_DepthTexture, g_PointSampler, uv, g_Constants.m_mInvView, g_Constants.m_mInvProj);

	float3 outRadiance = 0.0f;

	for (uint visPointLightIdx = 0; visPointLightIdx < gs_NumLightsPerTile; ++visPointLightIdx)
	{
		outRadiance += ShadePointLight(gbuffer, g_PointLights[gs_LightIdxPerTile[visPointLightIdx]]);
	}

	g_LightingSurface[DTid.xy] = float4(outRadiance, 1);
}