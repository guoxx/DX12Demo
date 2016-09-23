#pragma once

class DX12GraphicManager;

class DX12Device
{
public:
	DX12Device();
	~DX12Device();

	ID3D12CommandQueue* CreateCopyCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAG flags, uint32_t nodeMask);
	ID3D12CommandQueue* CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAG flags, uint32_t nodeMask);

	ID3D12CommandAllocator* CreateCopyCommandAllocator();
	ID3D12CommandAllocator* CreateGraphicCommandAllocator();

	ID3D12CommandList* CreateCopyCommandList();
	ID3D12CommandList* CreateGraphicCommandList();

private:
	ComPtr<ID3D12Device> m_d3dDevice;
};

