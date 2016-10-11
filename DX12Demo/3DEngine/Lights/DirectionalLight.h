#pragma once

#include "ILight.h"

class DirectionalLight : public ILight
{
public:
	DirectionalLight();
	virtual ~DirectionalLight();

	void SetIrradiance(float r, float g, float b) { m_Irradiance = DirectX::XMFLOAT4{r, g, b, 0}; }
	DirectX::XMFLOAT4 GetIrradiance() const { return m_Irradiance; }

	void SetDirection(float x, float y, float z) { m_Direction = DirectX::XMFLOAT4{x, y, z, 0}; }
	DirectX::XMFLOAT4 GetDirection() const { return m_Direction; }

private:
	DirectX::XMFLOAT4 m_Irradiance;
	DirectX::XMFLOAT4 m_Direction;
};

