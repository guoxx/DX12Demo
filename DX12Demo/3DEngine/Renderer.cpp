#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "Scene.h"
#include "Model.h"

#include "Filters/Filter2D.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	m_SwapChain = std::make_shared<DX12SwapChain>(DX12GraphicManager::GetInstance()->GetDevice(), hwnd, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_SceneGBuffer0 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer0->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneGBuffer1 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer1->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneGBuffer2 = std::make_shared<DX12ColorSurface>();
	m_SceneGBuffer2->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	m_SceneDepthSurface = std::make_shared<DX12DepthSurface>();
	m_SceneDepthSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_D32_FLOAT, width, height);

	m_LightingSurface = std::make_shared<DX12ColorSurface>();
	m_LightingSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height);

	m_IdentityFilter2D = std::make_shared<Filter2D>(DX12GraphicManager::GetInstance()->GetDevice(), L"IdentityFilter2D.hlsl");
}

Renderer::~Renderer()
{
}

void Renderer::Render(const Camera* pCamera, Scene* pScene)
{
	RenderGBuffer(pCamera, pScene);
	DeferredLighting(pCamera, pScene);
	ResolveToSwapChain();
}

void Renderer::Flip()
{
	m_SwapChain->Flip();
	DX12GraphicManager::GetInstance()->Flip();
}

void Renderer::DeferredLighting(const Camera* pCamera, Scene* pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	for (auto directionalLight : pScene->GetDirectionalLights())
	{
	}
}

void Renderer::RenderGBuffer(const Camera* pCamera, Scene* pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

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
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_GENERIC_READ);
	pGfxContext->SetGraphicsRootDescriptorTable(0, m_SceneGBuffer0->GetSRV());
	m_IdentityFilter2D->Draw(pGfxContext);
	pGfxContext->ResourceTransitionBarrier(m_SceneGBuffer0.get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

