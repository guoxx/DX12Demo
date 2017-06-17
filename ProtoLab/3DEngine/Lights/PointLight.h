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

    void SetColor(float r, float g, float b);

    void SetRadiantPower(float p);

    DirectX::XMVECTOR GetRadiantPower() const;

	void SetRadius(float r);
	float GetRadius() const;

	void GetViewNearFar(float& zNear, float& zFar) const;
	void GetViewAndProjMatrix(AXIS axis, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const;

private:
	DirectX::XMFLOAT4 m_Color;
	float m_RadiantPower;
	float m_Radius;

    DirectX::XMFLOAT4X4 m_mView[AXIS_END];
    DirectX::XMFLOAT4X4 m_mProj[AXIS_END];
};

