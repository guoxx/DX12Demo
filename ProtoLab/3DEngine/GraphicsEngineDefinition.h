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
extern NumVar g_ToneMapExposure;

extern BoolVar g_RSMEnabled;
extern NumVar g_RSMSampleRadius;
extern NumVar g_RSMFactor;
extern NumVar g_RSMRadiusEnd;

extern BoolVar g_EVSMEnabled;
extern NumVar g_EVSMPositiveExponent;
extern NumVar g_EVSMNegativeExponent;
extern NumVar g_LightBleedingReduction;
extern NumVar g_VSMBias;
