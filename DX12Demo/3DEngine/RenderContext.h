#pragma once

#include "../DX12/DX12.h"
#include "3DEngineDefinition.h"
#include "RenderableSurfaceManager.h"


class Camera;
class PointLight;
class DirectionalLight;

class __declspec(align(16)) RenderContext
{
public:
	RenderContext();
	~RenderContext();

	const Camera* GetCamera() const { return m_Camera; }

	void SetCamera(const Camera* pCamera) { m_Camera = pCamera; }

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

	DX12DepthSurface* AcquireDepthSurfaceForDirectionalLight(DirectionalLight* pDirLight);

	std::array<DX12DepthSurface*, 6> AcquireDepthSurfaceForPointLight(PointLight* pPointLight);

	void ReleaseDepthSurfacesForAllLights();

private:
	ShadingConfiguration m_ShadingCfg;

	const Camera* m_Camera;

	DirectX::XMMATRIX m_mModel;
	DirectX::XMMATRIX m_mView;
	DirectX::XMMATRIX m_mProj;
	DirectX::XMMATRIX m_mViewProj;
	DirectX::XMMATRIX m_mModelViewProj;

	std::map<DirectionalLight*, RenderableSurfaceHandle> m_ShadowMapForDirLights;
	std::map<PointLight*, std::array<RenderableSurfaceHandle, 6>> m_ShadowMapForPointLights;
};

