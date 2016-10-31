float4 PlaneEquation(float3 p0, float3 p1, float3 p2)
{
	float3 v0 = p1 - p0;
	float3 v1 = p2 - p0;
	float3 n = normalize(cross(v0, v1));
	float t = - dot(n, p0);
	return float4(n, t);
}

bool SphereCulling(float4 frustumPlanes[6], float3 center, float radius)
{
	for (uint i = 0; i < 6; ++i)
	{
		float dis = dot(frustumPlanes[i].xyz, center) + frustumPlanes[i].w;
		if (dis > radius)
		{
			return true;
		}
	}
	return false;
}

uint LinearizeTileId(uint2 tileId, uint numTileX, uint numTileY)
{
	return tileId.x + tileId.y * numTileX;
}

uint LinearizeThreadId(uint2 threadId)
{
	return threadId.x + threadId.y * LIGHT_CULLING_NUM_THREADS_XY;
}
