#include "pch.h"
#include "DirectionalLight.h"


DirectionalLight::DirectionalLight()
{
}

DirectionalLight::~DirectionalLight()
{
}


void DirectionalLight::SetIrradiance(float r, float g, float b)
{
	r = DX::Clamp(r, 0.0f, 1.0f);
	g = DX::Clamp(g, 0.0f, 1.0f);
	b = DX::Clamp(b, 0.0f, 1.0f);
	m_Irradiance = DirectX::XMFLOAT4{r, g, b, 0};
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	DirectX::XMVECTOR dir{ x, y, z, 0 };
	DirectX::XMVECTOR normalizedDir = DirectX::XMVector4Normalize(dir);
	DirectX::XMStoreFloat4(&m_Direction, normalizedDir);
}
