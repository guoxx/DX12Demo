#include "pch.h"
#include "SphericalCoordinates.h"

SphericalCoordinates SphericalCoordinates::FromSphere(DirectX::XMVECTOR coord)
{
    DirectX::XMVECTOR normalizedCoord = DirectX::XMVector3Normalize(coord);

    // [0, PI]
    float zenith = std::acos(DirectX::XMVectorGetY(normalizedCoord));

    // [-PI/2, PI/2]
    float elevation = DirectX::XM_PIDIV2 - zenith;

    // [0, 2*PI]
    float azimuth = std::atan2(DirectX::XMVectorGetZ(normalizedCoord), DirectX::XMVectorGetX(normalizedCoord));
    if (azimuth < 0)
    {
        azimuth += DirectX::XM_2PI;
    }

    SphericalCoordinates sphericalCoord;
    sphericalCoord.m_Elevation = elevation;
    sphericalCoord.m_Azimuth= azimuth;
    return sphericalCoord;
}

SphericalCoordinates SphericalCoordinates::FromThetaAndPhi(float theta, float phi)
{
    assert(0 <= theta && theta <= DirectX::XM_PI);
    assert(0 <= phi && phi <= DirectX::XM_2PI);

    SphericalCoordinates sphericalCoord;
    sphericalCoord.m_Elevation = DirectX::XM_PIDIV2 - theta;
    sphericalCoord.m_Azimuth = phi;
    return sphericalCoord;
}

DirectX::XMVECTOR SphericalCoordinates::ToSphere(SphericalCoordinates coord)
{
    float y = std::cos(coord.GetZenith());
    float x = std::sin(coord.GetZenith()) * std::cos(coord.GetAzimuth());
    float z = std::sin(coord.GetZenith()) * std::sin(coord.GetAzimuth());
    return DirectX::XMVectorSet(x, y, z, 1);
}
