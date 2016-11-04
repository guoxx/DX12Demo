#include "pch.h"
#include "GraphicsEngineDefinition.h"

BoolVar g_TiledShading("Graphics/Tiled Shading", true);
BoolVar g_ToneMapping("Graphics/HDR/Tone Mapping", true);
BoolVar g_RSMEnabled("Graphics/RSM/Enable", false);
NumVar g_RSMSampleRadius("Graphics/RSM/Sample Radius", 0.005f, 0.0f, 1.0f, 0.001);
NumVar g_RSMFactor("Graphics/RSM/RSM Factor", 4.0f, 0.0f, 20.0f, 0.5);
NumVar g_RSMRadiusEnd("Graphics/RSM/Radius End", 200.0f, 0.0f, 4000.0f, 20);
NumVar g_ToneMapExposure("Graphics/HDR/Exposure", 0.0f, -10.0f, 10.0f);