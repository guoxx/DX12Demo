#include "pch.h"
#include "PointLight.h"

#include "../Camera.h"
#include "../Mesh.h"
#include "../Material.h"


PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::SetIntensity(float r, float g, float b)
{
	//r = DX::Clamp(r, 0.0f, 1.0f);
	//g = DX::Clamp(g, 0.0f, 1.0f);
	//b = DX::Clamp(b, 0.0f, 1.0f);
	m_Intensity= DirectX::XMFLOAT4{r, g, b, 0};
}

DirectX::XMFLOAT4 PointLight::GetIntensity() const
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

void PointLight::GetViewAndProjMatrix(const Camera* pCamera, AXIS axis, uint32_t shadowMapSize, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const
{
	// tricks for seamless cubemap filtering
	// http://www.gamedev.net/blog/73/entry-2005516-seamless-filtering-across-faces-of-dynamic-cube-map/
	float halfSize = shadowMapSize/1.0f;
	float fov = 2.0f * atanf(halfSize/(halfSize-0.5f));

	float zNear = 0.01f;
	// HACK: add border so that we can handle depth bias properly
	float zFar = m_RadiusEnd * 1.4;
	float aspect = 1.0f;
	assert(zFar > zNear);

	*mProj = DirectX::XMMatrixPerspectiveFovRH(fov, aspect, zNear, zFar);

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

	DirectX::XMVECTOR position = GetTranslation();
	*mView = DirectX::XMMatrixLookToRH(position, axes[axis], upDir[axis]);
}
