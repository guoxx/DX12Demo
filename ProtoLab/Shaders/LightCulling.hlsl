#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0) " \
", SRV(t0) " \
", UAV(u0)" \
RootSigEnd

HLSLConstantBuffer(LightCullingConstants, 0, g_Constants)

StructuredBuffer<PointLight> g_PointLights : register(t0);
RWStructuredBuffer<LightNode> g_LightNodes: register(u0);

groupshared uint gs_LightListPerTilePtr;
groupshared uint gs_LightIdxPerTile[MAX_LIGHT_NODES_PER_TILE];

uint AllocateLightNodeInLDS()
{
	uint v;
	InterlockedAdd(gs_LightListPerTilePtr, 1, v);
	return v;
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
		gs_LightListPerTilePtr = 0;
	}
	GroupMemoryBarrierWithGroupSync();

	float nearZ = 0.001f;
	float farZ = 1.0f;
	float leftX = (float)tileId.x * LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.x;
	float topY = 1.0f - (float)tileId.y * LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.y;
	float rightX = saturate(leftX + LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.x);
	float bottomY = saturate(topY - LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.y);
	leftX   = leftX   * 2 - 1;
	rightX  = rightX  * 2 - 1;
	topY    = topY    * 2 - 1;
	bottomY = bottomY * 2 - 1;

	// frustom points in ndc space
	float3 frustumPoints[8];
	frustumPoints[0] = float3( leftX,  bottomY, nearZ );
	frustumPoints[1] = float3( rightX, bottomY, nearZ );
	frustumPoints[2] = float3( rightX, topY,    nearZ );
	frustumPoints[3] = float3( leftX,  topY,    nearZ );
	frustumPoints[4] = float3( leftX,  bottomY, farZ );
	frustumPoints[5] = float3( rightX, bottomY, farZ );
	frustumPoints[6] = float3( rightX, topY,    farZ );
	frustumPoints[7] = float3( leftX,  topY,    farZ );

	float3 frustomPointsCS[8];
	[loop]
	for (int frustumPtIdx = 0; frustumPtIdx < 8; ++frustumPtIdx)
	{
		float4 p = mul(float4(frustumPoints[frustumPtIdx], 1.0f), g_Constants.m_mInvProj);
		frustomPointsCS[frustumPtIdx] = p.xyz / p.w;
	}

	float4 frustumPlanes[6];
	frustumPlanes[0] = PlaneEquation(frustomPointsCS[0], frustomPointsCS[3], frustomPointsCS[4]); // left
	frustumPlanes[1] = PlaneEquation(frustomPointsCS[5], frustomPointsCS[6], frustomPointsCS[1]); // right
	frustumPlanes[2] = PlaneEquation(frustomPointsCS[1], frustomPointsCS[2], frustomPointsCS[0]); // near
	frustumPlanes[3] = PlaneEquation(frustomPointsCS[4], frustomPointsCS[7], frustomPointsCS[5]); // far
	frustumPlanes[4] = PlaneEquation(frustomPointsCS[2], frustomPointsCS[6], frustomPointsCS[3]); // top
	frustumPlanes[5] = PlaneEquation(frustomPointsCS[5], frustomPointsCS[1], frustomPointsCS[4]); // bottom

	uint maxNumLights = min(g_Constants.m_NumPointLights, MAX_LIGHT_NODES_PER_TILE);
	[loop]
	for (uint lightIdx = linearThreadId; lightIdx < maxNumLights; lightIdx += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		float3 lightPositionWS = g_PointLights[lightIdx].m_PositionAndRadius.xyz;
		float3 lightPositionCS = mul(float4(lightPositionWS, 1.0f), g_Constants.m_mView).xyz;
		float lightRadius = g_PointLights[lightIdx].m_PositionAndRadius.w;

		if (!SphereCulling(frustumPlanes, lightPositionCS, lightRadius))
		{
			uint ptr = AllocateLightNodeInLDS();
			gs_LightIdxPerTile[ptr] = lightIdx;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	uint startOffset = linearTileId * MAX_LIGHT_NODES_PER_TILE;
	uint numVisibleLights = gs_LightListPerTilePtr;
	[loop]
	for (uint visLightIdx = linearThreadId; visLightIdx < numVisibleLights; visLightIdx += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		uint offset = startOffset + visLightIdx;
		g_LightNodes[offset].m_LightIndex = gs_LightIdxPerTile[visLightIdx];
	}

	if (linearThreadId == 0)
	{
		g_LightNodes[startOffset + numVisibleLights].m_LightIndex = LIGHT_NODE_INVALID;
	}
}