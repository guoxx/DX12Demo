#pragma once

class SphericalCoordinates
{
public:
    // Right hand coordinate
    // (1, 0, 0) -> North
    // (0, 1, 0) -> Zenith
    // (0, 0, 1) -> East
    static SphericalCoordinates FromSphere(DirectX::XMVECTOR coord);
    static SphericalCoordinates FromThetaAndPhi(float theta, float phi);
    static DirectX::XMVECTOR ToSphere(SphericalCoordinates coord);
    
    float GetZenith() const
    {
        return DirectX::XM_PIDIV2 - m_Elevation;
    }

    float GetElevation() const
    {
        return m_Elevation;
    }

    float GetAzimuth() const
    {
        return m_Azimuth;
    }

private:
    float m_Elevation;
    float m_Azimuth;
};
