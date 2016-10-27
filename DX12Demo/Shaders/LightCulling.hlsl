#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0) " \
", SRV(t0) " \
", UAV(u0)" \
RootSigEnd

struct Constants
{
	uint m_NumPointLights;
	uint m_NumTileX;
	uint m_NumTileY;
	uint m_Padding0;
	float4x4 m_mView;
	float4x4 m_mInvProj;
	float4 m_InvScreenSize;
};

struct PointLightParam
{
	struct ShapeSphere m_Shape;
};

ConstantBuffer<Constants> g_Constants : register(b0);
StructuredBuffer<PointLightParam> g_PointLights : register(t0);
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
	float3 frustumPoints[4];
	frustumPoints[0] = float3( leftX,  bottomY, farZ );
	frustumPoints[1] = float3( rightX, bottomY, farZ );
	frustumPoints[2] = float3( rightX, topY,    farZ );
	frustumPoints[3] = float3( leftX,  topY,    farZ );

	float3 frustomPointsCS[4];
	[loop]
	for (int frustumPtIdx = 0; frustumPtIdx < 4; ++frustumPtIdx)
	{
		float4 p = mul(float4(frustumPoints[frustumPtIdx], 1.0f), g_Constants.m_mInvProj);
		frustomPointsCS[frustumPtIdx] = p.xyz / p.w;
	}

	float4 frustumPlanes[4];
	frustumPlanes[0] = PlaneEquation(float3(0, 0, 0), frustomPointsCS[0], frustomPointsCS[1]);
	frustumPlanes[1] = PlaneEquation(float3(0, 0, 0), frustomPointsCS[1], frustomPointsCS[2]);
	frustumPlanes[2] = PlaneEquation(float3(0, 0, 0), frustomPointsCS[2], frustomPointsCS[3]);
	frustumPlanes[3] = PlaneEquation(float3(0, 0, 0), frustomPointsCS[3], frustomPointsCS[0]);

	uint maxNumLights = min(g_Constants.m_NumPointLights, MAX_LIGHT_NODES_PER_TILE);
	[loop]
	for (uint lightIdx = linearThreadId; lightIdx < maxNumLights; lightIdx += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		float3 lightPositionWS = g_PointLights[lightIdx].m_Shape.m_Position_Radius.xyz;
		float3 lightPositionCS = mul(float4(lightPositionWS, 1.0f), g_Constants.m_mView).xyz;
		float lightRadius = g_PointLights[lightIdx].m_Shape.m_Position_Radius.w;

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
	for (uint visLightIdx = 0; visLightIdx < numVisibleLights; visLightIdx += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		uint offset = startOffset + visLightIdx;
		g_LightNodes[offset].m_LightIndex = gs_LightIdxPerTile[visLightIdx];
	}

	if (linearThreadId == 0)
	{
		g_LightNodes[startOffset + numVisibleLights].m_LightIndex = LIGHT_NODE_INVALID;
	}
}