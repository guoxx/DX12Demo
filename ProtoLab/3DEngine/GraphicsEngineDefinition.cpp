#include "pch.h"
#include "GraphicsEngineDefinition.h"

static const char* StrExposureModes[NumExposureModeEnum] = {
    "Saturation-based Speed",
    "Standard output sensitivity",
    "Automatic",
};

static const char* StrShutterSpeed[NumShutterSpeedEnum] = {
    "1s",
    "1/2s",
    "1/4s",
    "1/8s",
    "1/15s",
    "1/30s",
    "1/60s",
    "1/125s",
    "1/250s",
    "1/500s",
    "1/1000s",
    "1/2000s",
    "1/4000s",
};

static const char* StrFStops[NumFStopEnum] = {
    "f/1.8",
    "f/2.0",
    "f/2.2",
    "f/2.5",
    "f/2.8",
    "f/3.2",
    "f/3.5",
    "f/4.0",
    "f/4.5",
    "f/5.0",
    "f/5.6",
    "f/6.3",
    "f/7.1",
    "f/8.0",
    "f/9.0",
    "f/10.0",
    "f/11.0",
    "f/13.0",
    "f/14.0",
    "f/16.0",
    "f/18.0",
    "f/20.0",
    "f/22.0",
};

static const char* StrISORatings[NumISORatingEnum] = {
    "ISO100",
    "ISO200",
    "ISO400",
    "ISO800",
};


BoolVar g_TiledShading("Graphics/Tiled Shading", true);

BoolVar g_ToneMapping("Graphics/HDR/Tone Mapping", true);

BoolVar g_RSMEnabled("Graphics/RSM/Enable", false);
NumVar g_RSMSampleRadius("Graphics/RSM/Sample Radius", 0.005f, 0.0f, 1.0f, 0.001f);
NumVar g_RSMFactor("Graphics/RSM/RSM Factor", 4.0f, 0.0f, 20.0f, 0.5);
NumVar g_RSMRadiusEnd("Graphics/RSM/Radius End", 200.0f, 0.0f, 4000.0f, 20);

BoolVar g_EVSMEnabled("Graphics/EVSM/Enable", true);
NumVar g_EVSMPositiveExponent("Graphics/EVSM/Positive Exponent", 40.0000f, 0.0000f, 100.0000f, 0.1000f);
NumVar g_EVSMNegativeExponent("Graphics/EVSM/Negative Exponent", 5.0000f, 0.0000f, 100.0000f, 0.1000f);
NumVar g_LightBleedingReduction("Graphics/EVSM/Light Bleeding Reduction", 0.2000f, 0.0000f, 1.0000f, 0.0100f);
NumVar g_VSMBias("Graphics/EVSM/VSM Bias (x100)", 0.0100f, 0.0000f, 100.0000f, 0.0010f);

EnumVar g_ExposureMode("CameraControls/Expsure Mode", ManualExposure_SOS, NumExposureModeEnum, StrExposureModes);
EnumVar g_ShutterSpeed("CameraControls/Shutter Speed", ShutterSpeed1Over30, NumShutterSpeedEnum, StrShutterSpeed);
EnumVar g_Aperture("CameraControls/Aperture", FStop20Point0, NumFStopEnum, StrFStops);
EnumVar g_ISORating("CameraControls/ISO Rating", ISO100, NumISORatingEnum, StrISORatings);