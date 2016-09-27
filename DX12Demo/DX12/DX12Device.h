#pragma once

class DX12GraphicManager;

class DX12Device
{
public:
	DX12Device();
	~DX12Device();

	ID3D12CommandQueue* CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAGS flags, uint32_t nodeMask);

	ID3D12CommandAllocator* CreateGraphicCommandAllocator();

	ID3D12GraphicsCommandList* CreateGraphicCommandList(ID3D12CommandAllocator* allocator);

	ID3D12Resource* CreateCommittedResourceInDefaultHeap(uint64_t sizeInBytes, uint64_t alignInBytes, D3D12_RESOURCE_STATES initialState);

	ID3D12DescriptorHeap* CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* desc);

    uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType);

	ID3D12RootSignature* CreateRootSignature(const void* pBlobWithRootSignature, size_t blobLengthInBytes);

	ID3D12PipelineState* CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc);

	IDXGISwapChain1* CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_WHND hwnd);

	ID3D12Fence* CreateFence(uint64_t initialValue = 0);

    void CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
    void CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

private:
	ComPtr<ID3D12Device> m_d3dDevice;
};

