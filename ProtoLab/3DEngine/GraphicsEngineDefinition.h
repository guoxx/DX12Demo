#pragma once

#include "EngineTuning.h"
#include "Shaders/Inc/HLSLShared.h"
#include <stdlib.h>

enum ShadingConfiguration
{
	ShadingConfiguration_GBuffer = 0,
	ShadingConfiguration_DepthOnly,
	ShadingConfiguration_RSM,
	ShadingConfiguration_Max,
};

enum ExposureModeEnum
{
    ManualExposure_SBS = ExposureModes_Manual_SBS,
    ManualExposure_SOS = ExposureModes_Manual_SOS,
    AutomaticExposure = ExposureModes_Automatic,

    NumExposureModeEnum,
};

enum ShutterSpeedEnum
{
    ShutterSpeed1Over1 = 0,
    ShutterSpeed1Over2,
    ShutterSpeed1Over4,
    ShutterSpeed1Over8,
    ShutterSpeed1Over15,
    ShutterSpeed1Over30,
    ShutterSpeed1Over60,
    ShutterSpeed1Over125,
    ShutterSpeed1Over250,
    ShutterSpeed1Over500,
    ShutterSpeed1Over1000,
    ShutterSpeed1Over2000,
    ShutterSpeed1Over4000,

    NumShutterSpeedEnum
};

enum FStopEnum
{
    FStop1Point8 = 0,
    FStop2Point0,
    FStop2Point2,
    FStop2Point5,
    FStop2Point8,
    FStop3Point2,
    FStop3Point5,
    FStop4Point0,
    FStop4Point5,
    FStop5Point0,
    FStop5Point6,
    FStop6Point3,
    FStop7Point1,
    FStop8Point0,
    FStop9Point0,
    FStop10Point0,
    FStop11Point0,
    FStop13Point0,
    FStop14Point0,
    FStop16Point0,
    FStop18Point0,
    FStop20Point0,
    FStop22Point0,

    NumFStopEnum,
};

enum ISORatingEnum
{
    ISO100 = 0,
    ISO200,
    ISO400,
    ISO800,

    NumISORatingEnum
};

inline float GetApertureFNumber(int32_t v)
{
    static const float FNumbers[] =
    {
        1.8f, 2.0f, 2.2f, 2.5f, 2.8f, 3.2f, 3.5f, 4.0f, 4.5f, 5.0f, 5.6f, 6.3f, 7.1f, 8.0f,
        9.0f, 10.0f, 11.0f, 13.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f
    };
    static_assert(_countof(FNumbers) == NumFStopEnum, "");

    return FNumbers[int(v)];
}

inline float GetShutterSpeedValue(int32_t v)
{
    static const float ShutterSpeedValues[] =
    {
        1.0f / 1.0f, 1.0f / 2.0f, 1.0f / 4.0f, 1.0f / 8.0f, 1.0f / 15.0f, 1.0f / 30.0f,
        1.0f / 60.0f, 1.0f / 125.0f, 1.0f / 250.0f, 1.0f / 500.0f, 1.0f / 1000.0f, 1.0f / 2000.0f, 1.0f / 4000.0f,
    };
    static_assert(_countof(ShutterSpeedValues) == NumShutterSpeedEnum, "");

    return ShutterSpeedValues[int(v)];
}

inline float GetISORatingValue(int32_t v)
{
    static const float ISOValues[] =
    {
        100.0f, 200.0f, 400.0f, 800.0f
    };
    static_assert(_countof(ISOValues) == NumISORatingEnum, "");

    return ISOValues[int(v)];
}

extern BoolVar g_TiledShading;

extern BoolVar g_ToneMapping;
extern NumVar g_ToneMapTargetLuminance;

extern BoolVar g_RSMEnabled;
extern NumVar g_RSMSampleRadius;
extern NumVar g_RSMFactor;
extern NumVar g_RSMRadiusEnd;

extern BoolVar g_EVSMEnabled;
extern NumVar g_EVSMPositiveExponent;
extern NumVar g_EVSMNegativeExponent;
extern NumVar g_LightBleedingReduction;
extern NumVar g_VSMBias;

extern EnumVar g_ExposureMode;
extern EnumVar g_ShutterSpeed;
extern EnumVar g_Aperture;
extern EnumVar g_ISORating;
