#pragma once

#include "ILight.h"

class DirectionalLight : public ILight
{
public:
    enum 
    {
        NUM_CASCADED_SHADOW_MAP = 4
    };

	DirectionalLight();
	virtual ~DirectionalLight();

    void PrepareForShadowPass(const Camera* pCamera, uint32_t shadowMapSize);

	DirectX::XMFLOAT4 GetIrradiance() const { return m_Irradiance; }

	void SetDirection(float x, float y, float z);
	DirectX::XMFLOAT4 GetDirection() const { return m_Direction; }

	void GetViewAndProjMatrix(int32_t cascadeIdx, DirectX::XMMATRIX* mView, DirectX::XMMATRIX* mProj) const;

private:
    void UpdateLightingInfo();

    float m_PSSMSplitWeight;

    float m_Turbidity;
	DirectX::XMFLOAT4 m_Irradiance;
	DirectX::XMFLOAT4 m_Direction;

    DirectX::XMFLOAT4X4 m_mView[NUM_CASCADED_SHADOW_MAP];
    DirectX::XMFLOAT4X4 m_mProj[NUM_CASCADED_SHADOW_MAP];
};

