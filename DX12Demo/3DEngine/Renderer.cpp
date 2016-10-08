#include "pch.h"
#include "Renderer.h"


Renderer::Renderer(GFX_HWND hwnd, int32_t width, int32_t height)
	: m_Width{ width}
	, m_Height{ height }
{
	m_SwapChain = std::make_shared<DX12SwapChain>(DX12GraphicManager::GetInstance()->GetDevice(), hwnd, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
}

Renderer::~Renderer()
{
}

void Renderer::Render(Scene* pScene)
{
	ResolveToSwapChain();
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

void Renderer::Flip()
{
	m_SwapChain->Flip();
}
