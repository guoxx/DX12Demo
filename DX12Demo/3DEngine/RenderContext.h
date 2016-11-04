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

	void SetModelMatrix(DirectX::FXMMATRIX mModel)
	{
		m_mModel = mModel;

		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
	}

	void SetViewMatrix(DirectX::FXMMATRIX mView)
	{
		m_mView = mView;

		m_mViewProj = DirectX::XMMatrixMultiply(m_mView, m_mProj);
		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
	}
	
	void SetProjMatrix(DirectX::FXMMATRIX mProj)
	{
		m_mProj = mProj;

		m_mViewProj = DirectX::XMMatrixMultiply(m_mView, m_mProj);
		m_mModelViewProj = DirectX::XMMatrixMultiply(m_mModel, m_mViewProj);
	}

	ShadingConfiguration GetShadingCfg() const { return m_ShadingCfg; }
	void SetShadingCfg(ShadingConfiguration shadingCfg) { m_ShadingCfg = shadingCfg; }

	DX12ColorSurface* AcquireRSMRadiantIntensitySurfaceForDirectionalLight(DirectionalLight* pDirLight);

	DX12ColorSurface* AcquireRSMNormalSurfaceForDirectionalLight(DirectionalLight* pDirLight);

	DX12DepthSurface* AcquireDepthSurfaceForDirectionalLight(DirectionalLight* pDirLight);

	std::array<DX12ColorSurface*, 6> AcquireRSMRadiantIntensitySurfaceForPointLight(PointLight* pPointLight);

	std::array<DX12ColorSurface*, 6> AcquireRSMNormalSurfaceForPointLight(PointLight* pPointLight);

	std::array<DX12DepthSurface*, 6> AcquireDepthSurfaceForPointLight(PointLight* pPointLight);

	void ReleaseDepthSurfacesForAllLights();

private:
	ShadingConfiguration m_ShadingCfg;

	const Camera* m_Camera;

	uint32_t m_ScreenWidth;
	uint32_t m_ScreenHeight;

	DirectX::XMMATRIX m_mModel;
	DirectX::XMMATRIX m_mView;
	DirectX::XMMATRIX m_mProj;
	DirectX::XMMATRIX m_mViewProj;
	DirectX::XMMATRIX m_mModelViewProj;

	std::map<DirectionalLight*, RenderableSurfaceHandle<DX12DepthSurface>> m_ShadowMapForDirLights;
	std::map<PointLight*, std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6>> m_ShadowMapForPointLights;

	std::map<DirectionalLight*, RenderableSurfaceHandle<DX12ColorSurface>> m_RSMRadiantIntensitySurfaceForDirLights;
	std::map<DirectionalLight*, RenderableSurfaceHandle<DX12ColorSurface>> m_RSMNormalSurfaceForDirLights;
	std::map<PointLight*, std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6>> m_RSMRadiantIntensitySurfaceForPointLights;
	std::map<PointLight*, std::array<RenderableSurfaceHandle<DX12ColorSurface>, 6>> m_RSMNormalSurfaceForPointLights;

	const ILight* m_CurrentLightForRSM;
};

