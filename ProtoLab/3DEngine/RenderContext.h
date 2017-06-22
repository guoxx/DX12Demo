#pragma once

#include "../DX12/DX12.h"
#include "GraphicsEngineDefinition.h"
#include "RenderableSurfaceManager.h"


class Camera;
class ILight;
class PointLight;
class DirectionalLight;

class __declspec(align(16)) RenderContext
{
public:
	RenderContext();
	~RenderContext();

	void SetScreenSize(uint32_t w, uint32_t h)
	{
		m_ScreenWidth = w;
		m_ScreenHeight = h;
	}

	uint32_t GetScreenWidth() const { return m_ScreenWidth; }

	uint32_t GetScreenHeight() const { return m_ScreenHeight; }

	const Camera* GetCamera() const { return m_Camera; }

	void SetCamera(const Camera* pCamera) { m_Camera = pCamera; }

	const ILight* GetCurrentLightForRSM() const { return m_CurrentLightForRSM; }	

	void SetCurrentLightForRSM(const ILight* pLight) { m_CurrentLightForRSM = pLight; }

	DirectX::XMMATRIX GetModelMatrix() const { return m_mModel; };

	DirectX::XMMATRIX GetViewMatrix() const { return m_mView; };
	
	DirectX::XMMATRIX GetProjMatrix() const { return m_mProj; };

	DirectX::XMMATRIX GetViewPorjMatrix() const { return m_mViewProj; }

	DirectX::XMMATRIX GetModelViewProjMatrix() const { return m_mModelViewProj; }

	DirectX::XMMATRIX GetModelViewProjMatrixWithJitter() const { return m_mModelViewProjWithJitter; }

	void SetModelMatrix(DirectX::FXMMATRIX mModel)
	{
		m_mModel = mModel;

		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
        m_mModelViewProjWithJitter = DirectX::XMMatrixMultiply(m_mModelViewProj, m_mJitter);
	}

	void SetViewMatrix(DirectX::FXMMATRIX mView)
	{
		m_mView = mView;

		m_mViewProj = DirectX::XMMatrixMultiply(m_mView, m_mProj);
		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
        m_mModelViewProjWithJitter = DirectX::XMMatrixMultiply(m_mModelViewProj, m_mJitter);
	}
	
	void SetProjMatrix(DirectX::FXMMATRIX mProj)
	{
		m_mProj = mProj;

		m_mViewProj = DirectX::XMMatrixMultiply(m_mView, m_mProj);
		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
        m_mModelViewProjWithJitter = DirectX::XMMatrixMultiply(m_mModelViewProj, m_mJitter);
	}

	void SetJitter(float jitterX, float jitterY)
	{
        m_JitterOffset = DirectX::XMVectorSet(jitterX, jitterY, 0, 0);
        m_mJitter = DirectX::XMMatrixTranslation(jitterX * 2.0f / m_ScreenWidth, jitterY * 2.0f / m_ScreenHeight, 0);

        m_mModelViewProjWithJitter = DirectX::XMMatrixMultiply(m_mModelViewProj, m_mJitter);
	}

    XMVECTOR GetJitterOffset() const
	{
	    return m_JitterOffset;
	}

    void SaveInfoForNextFrame()
	{
	    m_JitterOffsetLastFrame = m_JitterOffset;
	    m_mViewLastFrame = m_mView;
	    m_mProjLastFrame = m_mProj;
	    m_mJitterLastFrame = m_mJitter;
	    m_mViewProjLastFrame = m_mViewProj;
	}

	ShadingConfiguration GetShadingCfg() const { return m_ShadingCfg; }
	void SetShadingCfg(ShadingConfiguration shadingCfg) { m_ShadingCfg = shadingCfg; }

	RenderableSurfaceHandle<DX12ColorSurface> AcquireEVSMSurfaceForDirectionalLight(const DirectionalLight* pDirLight);

	RenderableSurfaceHandle<DX12ColorSurface> AcquireRSMRadiantIntensitySurfaceForDirectionalLight(const DirectionalLight* pDirLight);

	RenderableSurfaceHandle<DX12ColorSurface> AcquireRSMNormalSurfaceForDirectionalLight(const DirectionalLight* pDirLight);

	RenderableSurfaceHandle<DX12DepthSurface> AcquireDepthSurfaceForDirectionalLight(const DirectionalLight* pDirLight);

	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> AcquireEVSMSurfaceForPointLight(const PointLight* pPointLight);

	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> AcquireRSMRadiantIntensitySurfaceForPointLight(const PointLight* pPointLight);

	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6> AcquireRSMNormalSurfaceForPointLight(const PointLight* pPointLight);

	std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6> AcquireDepthSurfaceForPointLight(const PointLight* pPointLight);

	void ReleaseDepthSurfacesForAllLights();

private:
	ShadingConfiguration m_ShadingCfg;

	const Camera* m_Camera;

	uint32_t m_ScreenWidth;
	uint32_t m_ScreenHeight;

    DirectX::XMVECTOR m_JitterOffset;

	DirectX::XMMATRIX m_mModel;
	DirectX::XMMATRIX m_mView;
	DirectX::XMMATRIX m_mProj;
	DirectX::XMMATRIX m_mJitter;
	DirectX::XMMATRIX m_mViewProj;
	DirectX::XMMATRIX m_mModelViewProj;
	DirectX::XMMATRIX m_mModelViewProjWithJitter;

	std::map<const DirectionalLight*, RenderableSurfaceHandle<DX12DepthSurface>> m_ShadowMapForDirLights;
	std::map<const PointLight*, std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6>> m_ShadowMapForPointLights;

	std::map<const DirectionalLight*, RenderableSurfaceHandle<DX12ColorSurface>> m_EVSMSurfaceForDirLights;
	std::map<const DirectionalLight*, RenderableSurfaceHandle<DX12ColorSurface>> m_RSMRadiantIntensitySurfaceForDirLights;
	std::map<const DirectionalLight*, RenderableSurfaceHandle<DX12ColorSurface>> m_RSMNormalSurfaceForDirLights;
	std::map<const PointLight*, std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6>> m_EVSMSurfaceForPointLights;
	std::map<const PointLight*, std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6>> m_RSMRadiantIntensitySurfaceForPointLights;
	std::map<const PointLight*, std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6>> m_RSMNormalSurfaceForPointLights;

	const ILight* m_CurrentLightForRSM;

public:
    DirectX::XMVECTOR m_JitterOffsetLastFrame;
	DirectX::XMMATRIX m_mViewLastFrame;
	DirectX::XMMATRIX m_mProjLastFrame;
	DirectX::XMMATRIX m_mJitterLastFrame;
	DirectX::XMMATRIX m_mViewProjLastFrame;
};

