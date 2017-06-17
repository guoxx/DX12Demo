#ifndef __POINT_LIGHT_HLSLI__
#define __POINT_LIGHT_HLSLI__

#include "Common.hlsli"

float PointLightDistanceFalloff(float distance, float radius)
{
	// Falloff function proposed by UE4, Real Shading in Unreal Engine 4
    // http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf p12
	float falloff = pow(saturate(1 - pow((distance/radius), 4)), 2) / (distance * distance + 1);
	return falloff;
}

float3 PointLightIrradiance(float3 radiantPower, float distance, float radius)
{
	float3 intensity = radiantPower / (4.0f * PI);
    float3 irradiance = intensity * PointLightDistanceFalloff(distance, radius);
	return irradiance;	
}

int GetFaceOfPointLightShadowMap(float3 lightPosition, float3 position)
{
	float3 LightVector = position - lightPosition;
	float3 AbsLightVector = abs(LightVector);
	float MaxCoordinate = max(AbsLightVector.x, max(AbsLightVector.y, AbsLightVector.z));
	int CubeFaceIndex = 0;
	if (MaxCoordinate == AbsLightVector.x)
	{
		CubeFaceIndex = AbsLightVector.x == LightVector.x ? 0 : 1;
	}
	else if (MaxCoordinate == AbsLightVector.y)
	{
		CubeFaceIndex = AbsLightVector.y == LightVector.y ? 2 : 3;
	}
	else
	{
		// we use right hand coordinate
		CubeFaceIndex = AbsLightVector.z == LightVector.z ? 5 : 4;
	}
	return CubeFaceIndex;
}

#endif