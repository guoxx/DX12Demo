#pragma once

class DX12GraphicManager;

class DX12Device
{
public:
	DX12Device();
	~DX12Device();

	ID3D12CommandQueue* CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAG flags, uint32_t nodeMask);

	ID3D12CommandAllocator* CreateGraphicCommandAllocator();

	ID3D12GraphicsCommandList* CreateGraphicCommandList();

	ID3D12Resource* CreateBuffer();

	ID3D12RootSignature* CreateRootSignature(const void* pBlobWithRootSignature, size_t blobLengthInBytes);

private:
	ComPtr<ID3D12Device> m_d3dDevice;
};

