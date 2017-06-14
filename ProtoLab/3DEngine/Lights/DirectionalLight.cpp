#include "pch.h"
#include "DirectionalLight.h"

#include "../Camera.h"


DirectionalLight::DirectionalLight()
    : m_PSSMSplitWeight{0.8f}
{
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::PrepareForShadowPass(const Camera* pCamera)
{
	const DirectX::XMVECTOR eyePos = DirectX::XMVectorZero();
	const DirectX::XMVECTOR lightDir = DirectX::XMLoadFloat4(&m_Direction);
	DirectX::XMVECTOR upDir = DirectX::XMVECTOR{ 0, 1, 0, 0 };
    if (std::abs(DirectX::XMVectorGetX(DirectX::XMVector4Dot(lightDir, upDir))) > 0.99f)
	{
		upDir = DirectX::XMVECTOR{ 0, 0, 1, 0 };
	}

	const DirectX::XMMATRIX mLightViewTemp = DirectX::XMMatrixLookToRH(eyePos, lightDir, upDir);
	const DirectX::XMMATRIX mInvLightViewTemp = DirectX::XMMatrixInverse(nullptr, mLightViewTemp);

	const DirectX::XMMATRIX mCameraToWorld = pCamera->GetInvViewMatrix();
	const DirectX::XMMATRIX mCameraToLight = DirectX::XMMatrixMultiply(mCameraToWorld, mLightViewTemp);

	const float tanFovY = std::tan(DirectX::XMConvertToRadians(pCamera->GetFovY() * 0.5f));
	const float aspectRatio = pCamera->GetAspectRatio();

	const float nearZ = pCamera->GetNear();
	const float farZ = pCamera->GetFar();

    float splitDistances[NUM_CASCADED_SHADOW_MAP + 1];
    splitDistances[0] = nearZ;
    splitDistances[NUM_CASCADED_SHADOW_MAP] = farZ;
    for (int32_t i = 1; i <= NUM_CASCADED_SHADOW_MAP; ++i)
    {
        float distLog = nearZ * std::pow(farZ / nearZ, static_cast<float>(i) / NUM_CASCADED_SHADOW_MAP);
        float distUniform = nearZ + (farZ - nearZ) * static_cast<float>(i) / NUM_CASCADED_SHADOW_MAP;
        float dist = lerp(distUniform, distLog, m_PSSMSplitWeight);
        splitDistances[i] = dist;
    }

    for (int32_t cascadeIdx = 0; cascadeIdx < NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
    {
        float localNearZ = splitDistances[cascadeIdx];
        float localFarZ = splitDistances[cascadeIdx + 1];

        float nearY = localNearZ * tanFovY;
        float nearX = nearY * aspectRatio;

        float farY = localFarZ * tanFovY;
        float farX = farY * aspectRatio;

        DirectX::XMVECTOR frustumPoints[8];
        frustumPoints[0] = DirectX::XMVECTOR{-nearX, -nearY, -localNearZ, 1}; // Near Left Bottom
        frustumPoints[1] = DirectX::XMVECTOR{nearX, -nearY, -localNearZ, 1}; // Near Right Bottom
        frustumPoints[2] = DirectX::XMVECTOR{nearX, nearY, -localNearZ, 1}; // Near Right Top
        frustumPoints[3] = DirectX::XMVECTOR{-nearX, nearY, -localNearZ, 1}; // Near Left Top
        frustumPoints[4] = DirectX::XMVECTOR{-farX, -farY, -localFarZ, 1}; // Far Left Bottom
        frustumPoints[5] = DirectX::XMVECTOR{farX, -farY, -localFarZ, 1}; // Far Right Bottom
        frustumPoints[6] = DirectX::XMVECTOR{farX, farY, -localFarZ, 1}; // Far Right Top
        frustumPoints[7] = DirectX::XMVECTOR{-farX, farY, -localFarZ, 1}; // Far Left Top

        DirectX::XMVECTOR aabbMin = DirectX::XMVectorReplicate(std::numeric_limits<float>::max());
        DirectX::XMVECTOR aabbMax = DirectX::XMVectorReplicate(-std::numeric_limits<float>::max());
        for (int32_t i = 0; i < _countof(frustumPoints); ++i)
        {
            DirectX::XMVECTOR p = DirectX::XMVector4Transform(frustumPoints[i], mCameraToLight);
            aabbMin = DirectX::XMVectorMin(aabbMin, p);
            aabbMax = DirectX::XMVectorMax(aabbMax, p);
        }

        DirectX::XMVECTOR centerPos = DirectX::XMVectorAdd(aabbMin, aabbMax);
        centerPos = DirectX::XMVectorMultiply(centerPos, DirectX::XMVECTOR{0.5, 0.5, 0.5, 0});
        centerPos = DirectX::XMVectorSetW(centerPos, 1.0f);
        DirectX::XMVECTOR centerPosWS = DirectX::XMVector4Transform(centerPos, mInvLightViewTemp);
        DirectX::XMMATRIX mView = DirectX::XMMatrixLookToRH(centerPosWS, lightDir, upDir);

        float depthBound = 4000;
        DirectX::XMVECTOR dim = DirectX::XMVectorSubtract(aabbMax, aabbMin);
        DirectX::XMMATRIX mProj = DirectX::XMMatrixOrthographicRH(DirectX::XMVectorGetX(dim), DirectX::XMVectorGetY(dim), -depthBound, depthBound);

        DirectX::XMStoreFloat4x4(&m_mView[cascadeIdx], mView);
        DirectX::XMStoreFloat4x4(&m_mProj[cascadeIdx], mProj);
    }
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

void DirectionalLight::GetViewAndProjMatrix(int32_t cascadeIdx, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const
{
    assert(cascadeIdx >= 0 && cascadeIdx < NUM_CASCADED_SHADOW_MAP);

    *mView = DirectX::XMLoadFloat4x4(&m_mView[cascadeIdx]);
    *mProj = DirectX::XMLoadFloat4x4(&m_mProj[cascadeIdx]);
}
