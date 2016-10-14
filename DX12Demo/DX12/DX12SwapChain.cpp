#include "pch.h"
#include "DX12SwapChain.h"

#include "DX12Device.h"
#include "DX12ColorSurface.h"
#include "DX12GraphicContextAutoExecutor.h"

DX12SwapChain::DX12SwapChain(DX12Device* device, const GFX_HWND hwnd, uint32_t backBufferWidth, uint32_t backBufferHeight, DXGI_FORMAT backBufferFormat)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = backBufferWidth;
	swapChainDesc.Height = backBufferHeight;
	swapChainDesc.Format = backBufferFormat;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = DX12NumSwapChainBuffers;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

#ifdef _XBOX_ONE
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = DXGIX_SWAP_CHAIN_MATCH_XBOX360_AND_PC;
#else
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
#endif

#ifdef _XBOX_ONE
	m_SwapChain = device->CreateSwapChain(&swapChainDesc, hwnd);
#else
	m_SwapChain = device->CreateSwapChain(&swapChainDesc, hwnd, DX12GraphicManager::GetInstance()->GetSwapChainCommandQueue());
#endif

	m_BackBufferIdx = 0;
	for (uint32_t i = 0; i < m_BackBuffers.size(); ++i)
	{
		ComPtr<ID3D12Resource> backBuffer;
        DX::ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_GRAPHICS_PPV_ARGS(backBuffer.GetAddressOf())));

		DX::SetNameIndexed(backBuffer.Get(), L"SWAP CHAIN BUFFER", i);

		m_BackBuffers[i] = std::make_shared<DX12ColorSurface>();
		m_BackBuffers[i]->InitAs2dSurface(device, backBuffer, D3D12_RESOURCE_STATE_COMMON);
	}
}

DX12SwapChain::~DX12SwapChain()
{
}

void DX12SwapChain::Begin()
{
	DX12SwapChainContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(GetBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
}

DX12ColorSurface * DX12SwapChain::GetBackBuffer() const
{
	return m_BackBuffers[m_BackBufferIdx].get();
}

void DX12SwapChain::Flip()
{
	{
		DX12SwapChainContextAutoExecutor executor;
		DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

		pGfxContext->ResourceTransitionBarrier(GetBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
	}

	m_SwapChain->Present(1, 0);

	m_BackBufferIdx = (m_BackBufferIdx + 1) % DX12NumSwapChainBuffers;
}
