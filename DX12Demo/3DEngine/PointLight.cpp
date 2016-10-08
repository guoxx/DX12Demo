#include "pch.h"
#include "PointLight.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"


PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::SetIntensity(DirectX::XMFLOAT3 intensity)
{
	m_Intensity = intensity;
}

DirectX::XMFLOAT3 PointLight::GetIntensity() const
{
	return m_Intensity;
}

void PointLight::SetRadius(float rStart, float rEnd)
{
	m_RadiusStart = rStart;
	m_RadiusEnd = rEnd;
}

float PointLight::GetRadiusStart() const
{
	return m_RadiusStart;
}

float PointLight::GetRadiusEnd() const
{
	return m_RadiusEnd;
}

void PointLight::GetViewNearFar(float& zNear, float& zFar) const
{
	zNear = 0.01f;
	zFar = m_RadiusEnd;
}

DirectX::XMMATRIX PointLight::GetViewProj(AXIS axis, uint32_t shadowMapSize) const
{
	// tricks for seamless cubemap filtering
	// http://www.gamedev.net/blog/73/entry-2005516-seamless-filtering-across-faces-of-dynamic-cube-map/
	float halfSize = shadowMapSize/1.0f;
	float fov = 2.0f * atanf(halfSize/(halfSize-0.5f));

	float zNear = 0.01f;
	float zFar = m_RadiusEnd;
	float aspect = 1.0f;
	assert(zFar > zNear);

	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, zNear, zFar);

	static const DirectX::XMVECTOR axes[AXIS_END] = {
		{1.0f, 0.0f, 0.0f, 0.0f},		// POSITIVE_X
		{-1.0f, 0.0f, 0.0f, 0.0f},		// NEGATIVE_X
		{0.0f, 1.0f, 0.0f, 0.0f},		// POSITIVE_Y
		{0.0f, -1.0f, 0.0f, 0.0f},		// NEGATIVE_Y
		{0.0f, 0.0f, -1.0f, 0.0f},		// POSITIVE_Z
		{0.0f, 0.0f, 1.0f, 0.0f},		// NEGATIVE_Z
	};
	static const DirectX::XMVECTOR upDir[AXIS_END] = {
		{0.0f, 1.0f, 0.0f, 0.0f},		// POSITIVE_X
		{0.0f, 1.0f, 0.0f, 0.0f},		// NEGATIVE_X
		{0.0f, 0.0f, 1.0f, 0.0f},		// POSITIVE_Y
		{0.0f, 0.0f, -1.0f, 0.0f},		// NEGATIVE_Y
		{0.0f, 1.0f, 0.0f, 0.0f},		// POSITIVE_Z
		{0.0f, 1.0f, 0.0f, 0.0f},		// NEGATIVE_Z
	};

	DirectX::XMVECTOR position = GetPosition();
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookToRH(position, axes[axis], upDir[axis]);
	
	DirectX::XMMATRIX mViewProj = DirectX::XMMatrixMultiply(mView, mProj);
	return mViewProj;
}
