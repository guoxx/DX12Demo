#pragma once

class DX12GraphicManager;

class DX12Device
{
public:
	DX12Device();
	~DX12Device();

	ComPtr<ID3D12CommandQueue> CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAGS flags, uint32_t nodeMask);

	ComPtr<ID3D12CommandAllocator> CreateGraphicCommandAllocator();

	ComPtr<ID3D12GraphicsCommandList> CreateGraphicCommandList(ID3D12CommandAllocator* allocator);

	ComPtr<ID3D12Heap> CreateHeap(uint64_t sizeInBytes,
		uint64_t alignInBytes,
		D3D12_HEAP_TYPE type,
		D3D12_CPU_PAGE_PROPERTY cpuPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL memoryPool = D3D12_MEMORY_POOL_UNKNOWN,
		D3D12_HEAP_FLAGS flags = D3D12_HEAP_FLAG_NONE);

	ComPtr<ID3D12Resource> CreatePlacedResource(ID3D12Heap* pHeap,
		uint64_t heapOffsetInBytes,
		const D3D12_RESOURCE_DESC* pDesc,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr);

	ComPtr<ID3D12Resource> CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties,
		D3D12_HEAP_FLAGS heapFlags,
		const D3D12_RESOURCE_DESC* pResourceDesc,
		D3D12_RESOURCE_STATES initialState,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue);

	ComPtr<ID3D12Resource> CreateCommittedBufferInDefaultHeap(uint64_t sizeInBytes, uint64_t alignInBytes, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);

	ComPtr<ID3D12Resource> CreateCommittedTexture2DInDefaultHeap(DXGI_FORMAT format,
		uint32_t width,
		uint32_t height,
		uint32_t arraySize,
		uint32_t mipLevels,
        uint32_t sampleCount = 1,
        uint32_t sampleQuality = 0,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON);

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* desc);

    uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType);

	ComPtr<ID3D12RootSignature> CreateRootSignature(const void* pBlobWithRootSignature, size_t blobLengthInBytes);

	ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc);

#ifdef _XBOX_ONE
	ComPtr<IDXGISwapChain1> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_HWND hwnd);
#else
	ComPtr<IDXGISwapChain1> CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_HWND hwnd, ID3D12CommandQueue* pQueue);
#endif

	ComPtr<ID3D12Fence> CreateFence(uint64_t initialValue = 0);

    void CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

    void CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);
        
    void CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor);

private:
	ComPtr<ID3D12Device> m_d3dDevice;
};

