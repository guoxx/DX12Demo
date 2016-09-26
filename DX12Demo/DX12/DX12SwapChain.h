#pragma once

class DX12Device;

class DX12SwapChain
{
public:
	DX12SwapChain(DX12Device* device, const GFX_WHND hwnd, uint32_t swapBufferCount, uint32_t backBufferWidth, uint32_t backBufferHeight, DXGI_FORMAT backBufferFormat);
	~DX12SwapChain();

private:
	const uint32_t m_SwapBufferCount;
	ComPtr<IDXGISwapChain1> m_SwapChain;
};

