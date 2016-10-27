#include "Inc/Common.hlsli"
#include "Inc/LightCulling.hlsli"

#define RootSigDeclaration \
RootSigBegin \
", CBV(b0) " \
", DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0))" \
RootSigEnd

struct Constants
{
	uint m_NumTileX;
	uint m_NumTileY;
};

struct PointLightParam
{
	struct ShapeSphere m_Shape;
};

ConstantBuffer<Constants> g_Constants : register(b0);
StructuredBuffer<PointLightParam> g_PointLights : register(t0);
StructuredBuffer<LightNode> g_LightNodes: register(t1);
RWTexture2D<float4> g_LightingSurface : register(u0);

groupshared uint gs_NumLightsPerTile = 0;
groupshared uint gs_LightIdxPerTile[MAX_LIGHT_NODES_PER_TILE];

[numthreads(LIGHT_CULLING_NUM_THREADS_XY, LIGHT_CULLING_NUM_THREADS_XY, 1)]
RootSigDeclaration
void CSMain(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	uint2 tileId = Gid.xy;
	uint linearTileId = LinearizeTileId(tileId, g_Constants.m_NumTileX, g_Constants.m_NumTileY);
	uint linearThreadId = LinearizeThreadId(GTid.xy);

	uint startOffset = linearTileId * MAX_LIGHT_NODES_PER_TILE;

	// TODO: HACK
	if (linearThreadId == 0)
	{
		for (uint i = 0; i < MAX_LIGHT_NODES_PER_TILE; ++i)
		{
			uint offset = startOffset + i;
			if (g_LightNodes[offset].m_LightIndex == MAX_LIGHT_NODES_PER_TILE)
			{
				break;
			}

			gs_LightIdxPerTile[i] = g_LightNodes[offset].m_LightIndex;
			gs_NumLightsPerTile += 1;
		}
	}

	GroupMemoryBarrierWithGroupSync();

	g_LightingSurface[DTid.xy] = float4(gs_NumLightsPerTile, 0, 0, 1);
}