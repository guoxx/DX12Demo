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

    void PrepareForShadowPass(const Camera* pCamera, uint32_t shadowMapSize);

	void SetIntensity(float r, float g, float b);
	DirectX::XMFLOAT4 GetIntensity() const;

	void SetRadius(float rStart, float rEnd);
	float GetRadiusStart() const;
	float GetRadiusEnd() const;

	void GetViewNearFar(float& zNear, float& zFar) const;
	void GetViewAndProjMatrix(AXIS axis, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const;

private:
	DirectX::XMFLOAT4 m_Intensity;
	float m_RadiusStart;
	float m_RadiusEnd;

    DirectX::XMFLOAT4X4 m_mView[AXIS_END];
    DirectX::XMFLOAT4X4 m_mProj[AXIS_END];
};

