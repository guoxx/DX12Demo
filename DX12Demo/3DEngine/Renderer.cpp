#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"

#include "Filters/Filter2D.h"
#include "Filters/DirectionalLightFilter2D.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	m_SwapChain = std::make_shared<DX12SwapChain>(DX12GraphicManager::GetInstance()->GetDevice(), hwnd, width, height, GFX_FORMAT_R8G8B8A8_UNORM_SWAPCHAIN);

	m_SceneGBuffer0 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer0->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneGBuffer1 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer1->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneGBuffer2 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer2->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneDepthSurface = std::make_shared<DX12DepthSurface>();
	m_SceneDepthSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_D32_FLOAT, width, height);

	m_LightingSurface = std::make_shared<DX12ColorSurface>();
	m_LightingSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_R32G32B32A32_FLOAT, width, height);

	m_ShadowMap_DirLight0 = std::make_shared<DX12DepthSurface>();
	m_ShadowMap_DirLight0->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_D32_FLOAT, 2048, 2048);

	for (int32_t i = 0; i < 6; ++i)
	{
		m_ShadowMap_PointLight0[i] = std::make_shared<DX12DepthSurface>();
		m_ShadowMap_PointLight0[i]->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), GFX_FORMAT_D32_FLOAT, 1024, 1024);
	}

	m_IdentityFilter2D = std::make_shared<Filter2D>(DX12GraphicManager::GetInstance()->GetDevice(), L"IdentityFilter2D.hlsl");

	m_DirLightFilter2D = std::make_shared<DirectionalLightFilter2D>(DX12GraphicManager::GetInstance()->GetDevice());
}

Renderer::~Renderer()
{
}

void Renderer::Render(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetCamera(pCamera);

	RenderShadowMaps(pCamera, pScene);
	RenderGBuffer(pCamera, pScene);
	DeferredLighting(pCamera, pScene);
	ResolveToSwapChain();
}

void Renderer::Flip()
{
	m_SwapChain->Flip();
	DX12GraphicManager::GetInstance()->Flip();
}

void Renderer::RenderShadowMaps(const Camera* pCamera, Scene * pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_DepthOnly);

		pGfxContext->PIXBeginEvent(L"ShadowMap - DirectionalLight");

		pGfxContext->ClearDepthTarget(m_ShadowMap_DirLight0.get(), 1.0f);
		pGfxContext->SetRenderTargets(0, nullptr, m_ShadowMap_DirLight0.get());
		pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pGfxContext->SetViewport(0, 0, 2048, 2048);

		m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
		DirectX::XMMATRIX mLightView;
		DirectX::XMMATRIX mLightProj;
		directionalLight->GetViewAndProjMatrix(pCamera, &mLightView, &mLightProj);
		m_RenderContext.SetViewMatrix(mLightView);
		m_RenderContext.SetProjMatrix(mLightProj);

		for (auto model : pScene->GetModels())
		{
			model->DrawPrimitives(&m_RenderContext, pGfxContext);
		}

		pGfxContext->PIXEndEvent();
	}

	for (auto pointLight : pScene->GetPointLights())
	{
		m_RenderContext.SetShadingCfg(ShadingConfiguration_DepthOnly);

		pGfxContext->PIXBeginEvent(L"ShadowMap - PointLight");

		for (int i = 0; i < 6; ++i)
		{
			wchar_t* axisNames[] = { L"POSITIVE_X", L"NEGATIVE_X", L"POSITIVE_Y", L"NEGATIVE_Y", L"POSITIVE_Z", L"NEGATIVE_Z" };
			pGfxContext->PIXBeginEvent(axisNames[i]);

			pGfxContext->ClearDepthTarget(m_ShadowMap_PointLight0[i].get(), 1.0f);
			pGfxContext->SetRenderTargets(0, nullptr, m_ShadowMap_PointLight0[i].get());
			pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			pGfxContext->SetViewport(0, 0, 1024, 1024);

			m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
			DirectX::XMMATRIX mLightView;
			DirectX::XMMATRIX mLightProj;
			pointLight->GetViewAndProjMatrix(pCamera, (PointLight::AXIS)i, 1024, &mLightView, &mLightProj);
			m_RenderContext.SetViewMatrix(mLightView);
			m_RenderContext.SetProjMatrix(mLightProj);

			for (auto model : pScene->GetModels())
			{
				model->DrawPrimitives(&m_RenderContext, pGfxContext);
			}

			pGfxContext->PIXEndEvent();
		}
		pGfxContext->PIXEndEvent();
	}
}

void Renderer::DeferredLighting(const Camera* pCamera, Scene* pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->PIXBeginEvent(L"DeferredLighting");

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = { m_LightingSurface.get() };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->ResourceTransitionBarrier(m_ShadowMap_DirLight0.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	for (int i = 0; i < 6; ++i)
	{
		pGfxContext->ResourceTransitionBarrier(m_ShadowMap_PointLight0[i].get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	PointLight* pPointLight = nullptr;
	if (pScene->GetPointLights().size() > 0)
	{
		pPointLight = pScene->GetPointLights()[0].get();
	}

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
		m_DirLightFilter2D->Apply(pGfxContext, &m_RenderContext, directionalLight.get(), pPointLight);

		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, m_SceneGBuffer0->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, m_SceneGBuffer1->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, m_SceneGBuffer2->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 3, m_SceneDepthSurface->GetStagingSRV().GetCpuHandle());
		pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 4, m_ShadowMap_DirLight0->GetStagingSRV().GetCpuHandle());
		for (int i = 0; i < 6; ++i)
		{
			pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 5 + i, m_ShadowMap_PointLight0[i]->GetStagingSRV().GetCpuHandle());
		}

		m_DirLightFilter2D->Draw(pGfxContext);
	}

	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer1.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer2.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	pGfxContext->ResourceTransitionBarrier(m_SceneDepthSurface.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	pGfxContext->ResourceTransitionBarrier(m_ShadowMap_DirLight0.get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	for (int i = 0; i < 6; ++i)
	{
		pGfxContext->ResourceTransitionBarrier(m_ShadowMap_PointLight0[i].get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	}

	pGfxContext->PIXEndEvent();
}

void Renderer::RenderGBuffer(const Camera* pCamera, Scene* pScene)
{
	m_RenderContext.SetShadingCfg(ShadingConfiguration_GBuffer);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->PIXBeginEvent(L"G-Buffer");

    // Clear the views.
    pGfxContext->ClearRenderTarget(m_SceneGBuffer0.get(), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(m_SceneGBuffer1.get(), 0, 0, 0, 0);
    pGfxContext->ClearRenderTarget(m_SceneGBuffer2.get(), 0, 0, 0, 0);
	pGfxContext->ClearDepthTarget(m_SceneDepthSurface.get(), 1.0f);

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = { m_SceneGBuffer0.get(), m_SceneGBuffer1.get(), m_SceneGBuffer2.get() };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_SceneDepthSurface.get());

	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_RenderContext.SetModelMatrix(DirectX::XMMatrixIdentity());
	m_RenderContext.SetViewMatrix(pCamera->GetViewMatrix());
	m_RenderContext.SetProjMatrix(pCamera->GetProjectionMatrix());

	for (auto model : pScene->GetModels())
	{
		model->DrawPrimitives(&m_RenderContext, pGfxContext);
	}

	pGfxContext->PIXEndEvent();
}

void Renderer::ResolveToSwapChain()
{
	m_SwapChain->Begin();

	DX12SwapChainContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	m_IdentityFilter2D->Apply(pGfxContext);
	pGfxContext->ResourceTransitionBarrier(m_LightingSurface.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->SetGraphicsRootDescriptorTable(0, m_LightingSurface->GetSRV());
	m_IdentityFilter2D->Draw(pGfxContext);
	pGfxContext->ResourceTransitionBarrier(m_LightingSurface.get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

