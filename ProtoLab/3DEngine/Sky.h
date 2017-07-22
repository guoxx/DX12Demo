#pragma once
#include "DX12/DX12Texture.h"

class SphericalCoordinates;

class Sky
{
public:
    Sky();

    std::shared_ptr<DX12Texture> GetEnvironmentMap() const { return m_EnvMap;  }

    void UpdateEnvironmentMap(float turbidity, Color groundAlbedo, SphericalCoordinates sunCoord);

private:
    std::shared_ptr<DX12Texture> m_EnvMap;
};
