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

void PointLight::PrepareForShadowPass(const Camera* pCamera, uint32_t shadowMapSize)
{
	// tricks for seamless cubemap filtering
	// http://www.gamedev.net/blog/73/entry-2005516-seamless-filtering-across-faces-of-dynamic-cube-map/
	float halfSize = shadowMapSize/1.0f;
	float fov = 2.0f * atanf(halfSize/(halfSize-0.5f));

	float zNear = 0.01f;
	// HACK: add border so that we can handle depth bias properly
	float zFar = m_Radius * 1.4f;
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

    for (int32_t axis = AXIS_START; axis < AXIS_END; ++axis)
    {
        DirectX::XMVECTOR position = GetTranslation();
        DirectX::XMMATRIX mView = DirectX::XMMatrixLookToRH(position, axes[axis], upDir[axis]);

        DirectX::XMStoreFloat4x4(&m_mView[axis], mView);
        DirectX::XMStoreFloat4x4(&m_mProj[axis], mProj);
    }
}

void PointLight::SetColor(float r, float g, float b)
{
	m_Color = DirectX::XMFLOAT4{r, g, b, 0};
}

void PointLight::SetRadiantPower(float p)
{
    m_RadiantPower = p;
}

DirectX::XMVECTOR PointLight::GetRadiantPower() const
{
    DirectX::XMVECTOR p = DirectX::XMLoadFloat4(&m_Color);
    return DirectX::XMVectorScale(p, m_RadiantPower);
}

void PointLight::SetRadius(float r)
{
	m_Radius = r;
}

float PointLight::GetRadius() const
{
	return m_Radius;
}

void PointLight::GetViewNearFar(float& zNear, float& zFar) const
{
	zNear = 0.01f;
	zFar = m_Radius;
}

void PointLight::GetViewAndProjMatrix(AXIS axis, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const
{
    *mView = DirectX::XMLoadFloat4x4(&m_mView[axis]);
    *mProj = DirectX::XMLoadFloat4x4(&m_mProj[axis]);
}
