#pragma once

#include "EngineTuning.h"

enum ShadingConfiguration
{
	ShadingConfiguration_GBuffer = 0,
	ShadingConfiguration_DepthOnly,
	ShadingConfiguration_RSM,
	ShadingConfiguration_Max,
};

extern BoolVar g_TiledShading;
extern BoolVar g_ToneMapping;
extern BoolVar g_RSMEnabled;
extern NumVar g_RSMSampleRadius;
extern NumVar g_RSMFactor;
extern NumVar g_RSMRadiusEnd;
extern NumVar g_ToneMapExposure;
