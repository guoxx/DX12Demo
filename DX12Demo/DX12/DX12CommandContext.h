#pragma once

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	~DX12CommandContext();

protected:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12CommandList> m_CommandList;
};

