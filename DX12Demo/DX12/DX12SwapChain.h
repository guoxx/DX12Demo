#pragma once

class DX12SwapChain
{
public:
	DX12SwapChain(DX12Device* device, uint32_t frameCount);
	~DX12SwapChain();
};

