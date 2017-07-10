#pragma once

#include "EngineTuning.h"

enum ShadingConfiguration
{
	ShadingConfiguration_GBuffer = 0,
	ShadingConfiguration_DepthOnly,
	ShadingConfiguration_RSM,
	ShadingConfiguration_Max,
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

enum FStopsEnum
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

    NumFStopsEnum,
};

enum ISORatingsEnum
{
    ISO100 = 0,
    ISO200,
    ISO400,
    ISO800,

    NumISORatingsEnum
};

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

extern EnumVar g_ShutterSpeed;
extern EnumVar g_Aperture;
extern EnumVar g_ISORating;
