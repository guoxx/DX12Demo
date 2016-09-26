#include "pch.h"
#include "DX12SwapChain.h"

#include "DX12Device.h"

DX12SwapChain::DX12SwapChain(DX12Device* device, const GFX_WHND hwnd, uint32_t backBufferWidth, uint32_t backBufferHeight, DXGI_FORMAT backBufferFormat)
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

	m_SwapChain = device->CreateSwapChain(&swapChainDesc, hwnd);
}

DX12SwapChain::~DX12SwapChain()
{
}

void DX12SwapChain::Flip()
{
	m_SwapChain->Present(1, 0);

	m_BackBufferIdx = (m_BackBufferIdx + 1) % DX12NumSwapChainBuffers;
}
