#pragma once

#include "ILight.h"

class DirectionalLight : public ILight
{
public:
	DirectionalLight();
	virtual ~DirectionalLight();

	void SetIrradiance(float r, float g, float b);
	DirectX::XMFLOAT4 GetIrradiance() const { return m_Irradiance; }

	void SetDirection(float x, float y, float z);
	DirectX::XMFLOAT4 GetDirection() const { return m_Direction; }

	void GetViewAndProjMatrix(const Camera* pCamera, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const;

private:
	DirectX::XMFLOAT4 m_Irradiance;
	DirectX::XMFLOAT4 m_Direction;
};

