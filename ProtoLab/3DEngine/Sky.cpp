#include "pch.h"
#include "Sky.h"
#include "SphericalCoordinates.h"
#include "Lights/HosekWilkie_SkylightModel/ArHosekSkyModel.h"
#include "DirectXTex/directxtex.h"

Sky::Sky()
{
}

void Sky::UpdateEnvironmentMap()
{
    SphericalCoordinates sunCoord = SphericalCoordinates::FromThetaAndPhi(DirectX::XMConvertToRadians(45), DirectX::XMConvertToRadians(45));
    DirectX::XMVECTOR sunDir = SphericalCoordinates::ToSphere(sunCoord);

    double turbidity = 2;
    double groundAlbedo[3] = {1, 1, 0};
    int32_t resolution = 2048;

    int32_t nTheta = resolution;
    int32_t nPhi = resolution * 2;

    ScratchImage img;
    img.Initialize2D(DXGI_FORMAT_R32G32B32A32_FLOAT, nPhi, nTheta, 1, 1);
    float* pixelsData = reinterpret_cast<float*>(img.GetImage(0, 0, 0)->pixels);

    auto storePixel = [&](int32_t x, int32_t y, float r, float g, float b)
    {
        int32_t idx = (x + y * nPhi) * 4;
        pixelsData[idx]     = r;
        pixelsData[idx + 1] = g;
        pixelsData[idx + 2] = b;
        pixelsData[idx + 3] = 1.0;
    };

    ArHosekSkyModelState* stateRGB[3];

    for (int i = 0; i < _countof(stateRGB); ++i)
    {
        stateRGB[i] = arhosek_rgb_skymodelstate_alloc_init(turbidity, groundAlbedo[i], sunCoord.GetElevation());
    }

    for (int32_t t = 0; t < nTheta; ++t)
    {
        float theta = (t + 0.5f) / nTheta * DirectX::XM_PI;

        if (theta > DirectX::XM_PIDIV2)
        {
            for (int32_t p = 0; p < nPhi; ++p)
            {
                storePixel(p, t, 0, 0, 0);
            }
            continue;
        }

        for (int32_t p = 0; p < nPhi; ++p)
        {
            float phi = (p + 0.5f) / nPhi * DirectX::XM_2PI;

            SphericalCoordinates sphericalCoord = SphericalCoordinates::FromThetaAndPhi(theta, phi);
            DirectX::XMVECTOR dir = SphericalCoordinates::ToSphere(sphericalCoord);

            float gamma = std::acos(DX::Clamp<float>(DirectX::XMVectorGetX(DirectX::XMVector3Dot(dir, sunDir)), -1, 1));

            float radiance[3];
            for (int c = 0; c < 3; ++c)
            {
                radiance[c] = arhosek_tristim_skymodel_radiance(stateRGB[c], theta, gamma, c);
                // Multiply by standard luminous efficacy of 683 lm/W to bring us in line with the photometric
                // units used during rendering
                // approx luminous efficiency as 0.29 
                radiance[c] *= 683.0f * 0.29;
            }

            storePixel(p, t, radiance[0], radiance[1], radiance[2]);
        }
    }
    
    for (int i = 0; i < _countof(stateRGB); ++i)
    {
        arhosekskymodelstate_free(stateRGB[i]);
    }
}
