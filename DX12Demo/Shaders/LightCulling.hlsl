#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", DescriptorTable(CBV(b0), SRV(t0), UAV(u0), visibility = SHADER_VISIBILITY_ALL)" \
RootSigEnd

struct Constants
{
	uint m_NumPointLights;
	uint m_NumTileX;
	uint m_NumTileY;
	float4x4 mInvViewProj;
	float4 m_InvScreenSize;
};

struct PointLightParam
{
	struct ShapeSphere m_Shape;
};

ConstantBuffer<Constants> g_Constants : register(b0);
StructuredBuffer<PointLightParam> g_PointLights : register(t0);
RWStructuredBuffer<LightNode> g_LightNodes: register(u0);

groupshared uint gs_LightListPerTilePtr = 0;
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

	float nearZ = 0.0f;
	float farZ = 1.0f;
	float leftX = (float)tileId.x * LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.x;
	float rightX = saturate(leftX + LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.x);
	float topY = 1.0f - (float)tileId.y * LIGHT_CULLING_NUM_THREADS_XY * g_Constants.m_InvScreenSize.y;
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

	float3 frustomPointsWS[8];
	for (int i = 0; i < 8; ++i)
	{
		float4 p = mul(float4(frustumPoints[i], 1.0f), g_Constants.mInvViewProj);
		frustomPointsWS[i] = p.xyz / p.w;
	}

	uint maxNumLights = min(g_Constants.m_NumPointLights, MAX_LIGHT_NODES_PER_TILE);
	for (uint lightIdx = linearThreadId; lightIdx < maxNumLights; lightIdx += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		if (!SphereCulling(frustomPointsWS, g_PointLights[lightIdx].m_Shape))
		{
			uint ptr = AllocateLightNodeInLDS();
			gs_LightIdxPerTile[ptr] = lightIdx;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	uint startOffset = linearTileId * MAX_LIGHT_NODES_PER_TILE;
	uint numVisibleLights = gs_LightListPerTilePtr;
	for (uint j = 0; j < numVisibleLights; j += NUM_THREADS_PER_LIGHT_CULLING_TILE)
	{
		uint offset = startOffset + i;
		g_LightNodes[offset].m_LightIndex = gs_LightIdxPerTile[i];
	}

	if (linearThreadId == 0)
	{
		g_LightNodes[numVisibleLights].m_LightIndex = LIGHT_NODE_INVALID;
	}
}