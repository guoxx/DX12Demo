#include "pch.h"
#include "GraphicsEngineDefinition.h"

BoolVar g_TiledShading("Graphics/Tiled Shading", true);

BoolVar g_ToneMapping("Graphics/HDR/Tone Mapping", true);
NumVar g_ToneMapExposure("Graphics/HDR/Exposure", 0.0f, -10.0f, 10.0f);

BoolVar g_RSMEnabled("Graphics/RSM/Enable", false);
NumVar g_RSMSampleRadius("Graphics/RSM/Sample Radius", 0.005f, 0.0f, 1.0f, 0.001);
NumVar g_RSMFactor("Graphics/RSM/RSM Factor", 4.0f, 0.0f, 20.0f, 0.5);
NumVar g_RSMRadiusEnd("Graphics/RSM/Radius End", 200.0f, 0.0f, 4000.0f, 20);

BoolVar g_EVSMEnabled("Graphics/EVSM/Enable", false);
NumVar g_EVSMPositiveExponent("Graphics/EVSM/Positive Exponent", 40.0000f, 0.0000f, 100.0000f, 0.1000f);
NumVar g_EVSMNegativeExponent("Graphics/EVSM/Negative Exponent", 5.0000f, 0.0000f, 100.0000f, 0.1000f);
NumVar g_LightBleedingReduction("Graphics/EVSM/Light Bleeding Reduction", 0.0000f, 0.0000f, 1.0000f, 0.0100f);
NumVar g_VSMBias("Graphics/EVSM/VSM Bias (x100)", 0.0100f, 0.0000f, 100.0000f, 0.0010f);