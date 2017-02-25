#include "pch.h"
#include "DirectionalLight.h"

#include "../Camera.h"


DirectionalLight::DirectionalLight()
{
}

DirectionalLight::~DirectionalLight()
{
}


void DirectionalLight::SetIrradiance(float r, float g, float b)
{
	//r = DX::Clamp(r, 0.0f, 1.0f);
	//g = DX::Clamp(g, 0.0f, 1.0f);
	//b = DX::Clamp(b, 0.0f, 1.0f);
	m_Irradiance = DirectX::XMFLOAT4{r, g, b, 0};
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	DirectX::XMVECTOR dir{ x, y, z, 0 };
	DirectX::XMVECTOR normalizedDir = DirectX::XMVector4Normalize(dir);
	DirectX::XMStoreFloat4(&m_Direction, normalizedDir);
}

void DirectionalLight::GetViewAndProjMatrix(const Camera* pCamera, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const
{
	DirectX::XMVECTOR eyePos = DirectX::XMVectorZero();
	DirectX::XMVECTOR upDir = DirectX::XMVECTOR{ 0, 1, 0, 0 };
	DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat4(&m_Direction);
    if (std::abs(DirectX::XMVectorGetX(DirectX::XMVector4Dot(lightDir, upDir))) > 0.99f)
	{
		upDir = DirectX::XMVECTOR{ 0, 0, 1, 0 };
	}

	DirectX::XMMATRIX mLightViewTemp = DirectX::XMMatrixLookToRH(eyePos, lightDir, upDir);
	DirectX::XMMATRIX mInvLightViewTemp = DirectX::XMMatrixInverse(nullptr, mLightViewTemp);

	DirectX::XMMATRIX mCameraToWorld = pCamera->GetInvViewMatrix();
	DirectX::XMMATRIX mCameraToLight = DirectX::XMMatrixMultiply(mCameraToWorld, mLightViewTemp);

	float tanFovY = std::tan(DirectX::XMConvertToRadians(pCamera->GetFovY() * 0.5f));
	float aspectRatio = pCamera->GetAspectRatio();

	float nearZ = pCamera->GetNear();
	float nearY = nearZ * tanFovY;
	float nearX = nearY * aspectRatio;

	float farZ = pCamera->GetFar();
	float farY = farZ * tanFovY;
	float farX = farY * aspectRatio;

	DirectX::XMVECTOR frustumPoints[8];
	frustumPoints[0] = DirectX::XMVECTOR{ -nearX, -nearY, -nearZ, 1 };   // Near Left Bottom
	frustumPoints[1] = DirectX::XMVECTOR{ nearX, -nearY, -nearZ, 1 };   // Near Right Bottom
	frustumPoints[2] = DirectX::XMVECTOR{ nearX, nearY, -nearZ, 1 };   // Near Right Top
	frustumPoints[3] = DirectX::XMVECTOR{ -nearX, nearY, -nearZ, 1 };   // Near Left Top
	frustumPoints[4] = DirectX::XMVECTOR{ -farX, -farY, -farZ, 1 };    // Far Left Bottom
	frustumPoints[5] = DirectX::XMVECTOR{ farX, -farY, -farZ, 1 };    // Far Right Bottom
	frustumPoints[6] = DirectX::XMVECTOR{ farX, farY, -farZ, 1 };    // Far Right Top
	frustumPoints[7] = DirectX::XMVECTOR{ -farX, farY, -farZ, 1 };    // Far Left Top

	DirectX::XMVECTOR aabbMin = DirectX::XMVECTOR{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	DirectX::XMVECTOR aabbMax = DirectX::XMVECTOR{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
	for (int32_t i = 0; i < _countof(frustumPoints); ++i)
	{
		DirectX::XMVECTOR p = DirectX::XMVector4Transform(frustumPoints[i], mCameraToLight);
		aabbMin = DirectX::XMVectorMin(aabbMin, p);
		aabbMax = DirectX::XMVectorMax(aabbMax, p);
	}
	
	DirectX::XMVECTOR centerPos = DirectX::XMVectorAdd(aabbMin, aabbMax);
	centerPos = DirectX::XMVectorMultiply(centerPos, DirectX::XMVECTOR{ 0.5, 0.5, 0.5, 0 });
	centerPos = DirectX::XMVectorSetW(centerPos, 1.0f);
	DirectX::XMVECTOR centerPosWS = DirectX::XMVector4Transform(centerPos, mInvLightViewTemp);
	*mView = DirectX::XMMatrixLookToRH(centerPosWS, lightDir, upDir);

	float depthBound = 4000;
	DirectX::XMVECTOR dim = DirectX::XMVectorSubtract(aabbMax, aabbMin);
	*mProj = DirectX::XMMatrixOrthographicRH(DirectX::XMVectorGetX(dim), DirectX::XMVectorGetY(dim), -depthBound, depthBound);
}
