#include "pch.h"
#include "Renderer.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	m_SwapChain = std::make_shared<DX12SwapChain>(DX12GraphicManager::GetInstance()->GetDevice(), hwnd, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_SceneColorSurface = std::make_shared<DX12ColorSurface>();
	m_SceneColorSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	
	m_SceneDepthSurface = std::make_shared<DX12DepthSurface>();
	m_SceneDepthSurface->InitAs2dSurface(DX12GraphicManager::GetInstance()->GetDevice(), DXGI_FORMAT_D32_FLOAT, width, height);
}

Renderer::~Renderer()
{
}

void Renderer::Render(const Camera* pCamera, Scene* pScene)
{
	RenderScene(pCamera, pScene);
	ResolveToSwapChain();
}

void Renderer::Flip()
{
	m_SwapChain->Flip();
}

void Renderer::RenderScene(const Camera* pCamera, Scene* pScene)
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

    // Clear the views.
    pGfxContext->ClearRenderTarget(m_SceneColorSurface.get(), 0, 0, 0, 0);
	pGfxContext->ClearDepthTarget(m_SceneDepthSurface.get(), 1.0f);

	// setup color and depth buffers
	DX12ColorSurface* pColorSurfaces[] = { m_SceneColorSurface.get() };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_SceneDepthSurface.get());
}

void Renderer::ResolveToSwapChain()
{
	m_SwapChain->Begin();

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, nullptr);
    pGfxContext->ClearRenderTarget(pColorSurface, 0, 1, 1, 1);
}

