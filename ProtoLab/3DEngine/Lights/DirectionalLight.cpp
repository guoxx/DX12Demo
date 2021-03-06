#include "pch.h"
#include "DirectionalLight.h"

#include "../Camera.h"
#include "3DEngine/Lights/SunModel.h"


DirectX::XMMATRIX GetCascadeRoundingMatrix(const DirectX::XMMATRIX& shadowViewProjMatrix, float shadowMapSize)
{
    // code from here https://www.gamedev.net/topic/497259-stable-cascaded-shadow-maps/
    DirectX::XMVECTOR origin = DirectX::XMVectorSet(0, 0, 0, 1);
    DirectX::XMVECTOR shadowOrigin = DirectX::XMVector4Transform(origin, shadowViewProjMatrix);
    shadowOrigin /= DirectX::XMVectorGetW(shadowOrigin);

    // Find nearest shadow map texel. The 0.5f is because x,y are in the 
    // range -1 .. 1 and we need them in the range 0 .. 1
    shadowOrigin = shadowOrigin * (shadowMapSize / 2.0f);

    DirectX::XMVECTOR roundedOrigin = DirectX::XMVectorRound(shadowOrigin);
    DirectX::XMVECTOR roundOffset = roundedOrigin - shadowOrigin;
    roundOffset = roundOffset * (2.0f / shadowMapSize);
    roundOffset = DirectX::XMVectorSetZ(roundOffset, 0);
    roundOffset = DirectX::XMVectorSetW(roundOffset, 0);

    DirectX::XMMATRIX translation = DirectX::XMMatrixTranslationFromVector(roundOffset);
    return translation;
}

/* Apparent radius of the sun as seen from the earth (in degrees).
This is an approximation--the actual value is somewhere between
0.526 and 0.545 depending on the time of year */
#define SUN_APP_RADIUS 0.5358

DirectionalLight::DirectionalLight()
    : m_PSSMSplitWeight{0.75f}
    , m_Turbidity{2.0f}
{
}

DirectionalLight::~DirectionalLight()
{
}

void DirectionalLight::PrepareForShadowPass(const Camera* pCamera, uint32_t shadowMapSize)
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
        DirectX::XMVECTOR frustumPointsInLightSpace[8];
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
            frustumPointsInLightSpace[i] = p;
        }

        DirectX::XMVECTOR centerPos = DirectX::XMVectorAdd(aabbMin, aabbMax);
        centerPos = DirectX::XMVectorMultiply(centerPos, DirectX::XMVECTOR{0.5, 0.5, 0.5, 0});
        centerPos = DirectX::XMVectorSetW(centerPos, 1.0f);
        DirectX::XMVECTOR centerPosWS = DirectX::XMVector4Transform(centerPos, mInvLightViewTemp);
        DirectX::XMMATRIX mView = DirectX::XMMatrixLookToRH(centerPosWS, lightDir, upDir);

        float sphereRadius = 0.0f;
        for (int32_t i = 0; i < _countof(frustumPoints); ++i)
        {
            float l = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(frustumPointsInLightSpace[i], centerPos)));
            sphereRadius = std::max(sphereRadius, l);
            sphereRadius = std::ceilf(sphereRadius / 2.0f) * 2.0f;
        }

        float depthBound = 100;
        DirectX::XMMATRIX mProj = DirectX::XMMatrixOrthographicRH(2.0f * sphereRadius, 2.0f * sphereRadius, -depthBound, depthBound);

        DirectX::XMMATRIX mViewProj = DirectX::XMMatrixMultiply(mView, mProj);
        DirectX::XMMATRIX mRounding = GetCascadeRoundingMatrix(mViewProj, shadowMapSize / 2.0f);

        DirectX::XMStoreFloat4x4(&m_mView[cascadeIdx], mView);
        DirectX::XMStoreFloat4x4(&m_mProj[cascadeIdx], DirectX::XMMatrixMultiply(mProj, mRounding));
    }
}

void DirectionalLight::SetDirection(float x, float y, float z)
{
	DirectX::XMVECTOR dir{ x, y, z, 0 };
	DirectX::XMVECTOR normalizedDir = DirectX::XMVector4Normalize(dir);
	DirectX::XMStoreFloat4(&m_Direction, normalizedDir);

    UpdateLightingInfo();
}

void DirectionalLight::GetViewAndProjMatrix(int32_t cascadeIdx, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const
{
    assert(cascadeIdx >= 0 && cascadeIdx < NUM_CASCADED_SHADOW_MAP);

    *mView = DirectX::XMLoadFloat4x4(&m_mView[cascadeIdx]);
    *mProj = DirectX::XMLoadFloat4x4(&m_mProj[cascadeIdx]);
}

void DirectionalLight::UpdateLightingInfo()
{
    float azimuth = std::atan2(-m_Direction.x, m_Direction.z);
    float elevation = std::acos(-m_Direction.y);
    if (azimuth < 0)
        azimuth += 2 * HLSL::PI;

    Math::SampledSpectrum sunRadianceSPD = computeSunRadiance(elevation, m_Turbidity);

    double sunRadiance = 0;
    double sunLuminance = 0;
    for (int i = 0; i < Math::NumSpectralSamples; ++i)
    {
        sunRadiance += sunRadianceSPD[i] * Math::SpectrumSamplesStep;
        sunLuminance += sunRadianceSPD[i] * 683 * Math::SampledSpectrum::Y[i] * Math::SpectrumSamplesStep;
    }
    double luminousEfficiency = sunLuminance / sunRadiance;

    float theta = DirectX::XMConvertToRadians(SUN_APP_RADIUS * 0.5f);
    float solidAngle = 2 * HLSL::PI * (1 - std::cos(theta));

    Math::RGBSpectrum radiance = sunRadianceSPD.ToRGBSpectrum();
    Math::RGBSpectrum luminance = radiance * Math::CIE_Y_integral * luminousEfficiency;
    Math::RGBSpectrum illuminance = luminance * solidAngle;

    m_Irradiance = float4{illuminance[0], illuminance[1], illuminance[2], 0};
}
