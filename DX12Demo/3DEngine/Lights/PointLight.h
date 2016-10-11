#pragma once

#include "ILight.h"

class PointLight : public ILight
{
public:
	enum AXIS
	{
		AXIS_START = 0,
		POSITIVE_X = AXIS_START,
		NEGATIVE_X,
		POSITIVE_Y,
		NEGATIVE_Y,
		POSITIVE_Z,
		NEGATIVE_Z,
		AXIS_END,
	};

	PointLight();
	virtual ~PointLight();

	void SetIntensity(float r, float g, float b);
	DirectX::XMFLOAT4 GetIntensity() const;

	void SetRadius(float rStart, float rEnd);
	float GetRadiusStart() const;
	float GetRadiusEnd() const;

	void GetViewNearFar(float& zNear, float& zFar) const;
	DirectX::XMMATRIX GetViewProj(AXIS axis, uint32_t shadowMapSize) const;

private:
	DirectX::XMFLOAT4 m_Intensity;
	float m_RadiusStart;
	float m_RadiusEnd;
};

