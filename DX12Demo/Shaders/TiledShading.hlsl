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

groupshared uint gs_LightListPerTilePtr = 0;
groupshared uint gs_LightIdxPerTile[MAX_LIGHT_NODES_PER_TILE];

[numthreads(LIGHT_CULLING_NUM_THREADS_XY, LIGHT_CULLING_NUM_THREADS_XY, 1)]
RootSigDeclaration
void CSMain(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	g_LightingSurface[DTid.xy] = float4(1, 0, 0, 1);
}