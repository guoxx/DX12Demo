#pragma once

#include "DX12Constants.h"
#include "DX12Fence.h"

class DX12Device;
class DX12ColorSurface;

class DX12SwapChain
{
public:
	DX12SwapChain(DX12Device* device, const GFX_HWND hwnd, uint32_t backBufferWidth, uint32_t backBufferHeight, DXGI_FORMAT backBufferFormat);
	~DX12SwapChain();

	void Begin();

	DX12ColorSurface* GetBackBuffer() const;

	void Flip();

private:
	ComPtr<IDXGISwapChain1> m_SwapChain;

	// TODO: add fence to detect if back buffer is been using by GPU
	uint32_t m_BackBufferIdx;
	std::array<std::shared_ptr<DX12ColorSurface>, DX12NumSwapChainBuffers> m_BackBuffers;
};

