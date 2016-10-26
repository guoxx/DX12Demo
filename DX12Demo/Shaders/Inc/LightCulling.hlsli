#define LIGHT_CULLING_NUM_THREADS_XY 16
#define NUM_THREADS_PER_LIGHT_CULLING_TILE (LIGHT_CULLING_NUM_THREADS_XY * LIGHT_CULLING_NUM_THREADS_XY)
#define MAX_LIGHT_NODES_PER_TILE 128
#define LIGHT_NODE_INVALID 0x7FFFFFFF


struct ShapeSphere
{
	float3 m_Position;
	float m_Radius;
};

struct LightNode
{
	int m_LightIndex;
};

bool SphereCulling(float3 frustumPoints[8], ShapeSphere sphere)
{
	for (uint i = 0; i < 8; ++i)
	{
		if (length(frustumPoints[i] - sphere.m_Position) <= sphere.m_Radius)
		{
			return false;
		}
	}
	return true;
}

uint LinearizeTileId(uint2 tileId, uint numTileX, uint numTileY)
{
	return tileId.x + tileId.y * numTileY;
}

uint LinearizeThreadId(uint2 threadId)
{
	return threadId.x + threadId.y * LIGHT_CULLING_NUM_THREADS_XY;
}
