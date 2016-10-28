#pragma once

enum ShadingConfiguration
{
	ShadingConfiguration_GBuffer = 0,
	ShadingConfiguration_DepthOnly,
	ShadingConfiguration_Max,
};

// Keep the same as LightCulling.hlsli
#define LIGHT_CULLING_NUM_THREADS_XY 8
#define NUM_THREADS_PER_LIGHT_CULLING_TILE (LIGHT_CULLING_NUM_THREADS_XY * LIGHT_CULLING_NUM_THREADS_XY)
#define MAX_LIGHT_NODES_PER_TILE 128
#define MAX_POINT_LIGHTS_PER_FRAME 1024
#define LIGHT_NODE_INVALID 0x7FFFFFFF
