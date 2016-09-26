#pragma once

#include "DX12Constants.h"

class DX12Device;

class DX12SwapChain
{
public:
	DX12SwapChain(DX12Device* device, const GFX_WHND hwnd, uint32_t backBufferWidth, uint32_t backBufferHeight, DXGI_FORMAT backBufferFormat);
	~DX12SwapChain();

	void Flip();

private:
	ComPtr<IDXGISwapChain1> m_SwapChain;

	uint32_t m_BackBufferIdx;
	std::array<ComPtr<ID3D12Fence>, DX12NumSwapChainBuffers> m_Fences;
	std::array<uint32_t, DX12NumSwapChainBuffers> m_FencesValue;
	std::array<ComPtr<ID3D12Resource>, DX12NumSwapChainBuffers> m_BackBuffers;
};

