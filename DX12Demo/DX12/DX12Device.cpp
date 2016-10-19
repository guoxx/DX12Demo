#include "pch.h"
#include "DX12Device.h"

DX12Device::DX12Device()
{
    DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_GRAPHICS_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())));
}

DX12Device::~DX12Device()
{
}

ComPtr<ID3D12CommandQueue> DX12Device::CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAGS flags, uint32_t nodeMask)
{
    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = priority;
    desc.Flags = flags;
	desc.NodeMask = nodeMask;

	ComPtr<ID3D12CommandQueue> pQueue;
    DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&desc, IID_GRAPHICS_PPV_ARGS(pQueue.GetAddressOf())));
	return pQueue;
}

ComPtr<ID3D12CommandAllocator> DX12Device::CreateGraphicCommandAllocator()
{
	ComPtr<ID3D12CommandAllocator> pAllocator;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(pAllocator.GetAddressOf())));
	return pAllocator;
}

ComPtr<ID3D12GraphicsCommandList> DX12Device::CreateGraphicCommandList(ID3D12CommandAllocator* allocator)
{
	ComPtr<ID3D12GraphicsCommandList> pCommandList;
    DX::ThrowIfFailed(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_GRAPHICS_PPV_ARGS(pCommandList.GetAddressOf())));
    pCommandList->Close();
	return pCommandList;
}

ComPtr<ID3D12Heap> DX12Device::CreateHeap(uint64_t sizeInBytes,
	uint64_t alignInBytes,
	D3D12_HEAP_TYPE type,
	D3D12_CPU_PAGE_PROPERTY cpuPageProperty,
	D3D12_MEMORY_POOL memoryPool,
	D3D12_HEAP_FLAGS flags)
{
	D3D12_HEAP_DESC desc;
	desc.SizeInBytes = sizeInBytes;
	desc.Properties.Type = type;
	desc.Properties.CPUPageProperty = cpuPageProperty;
	desc.Properties.MemoryPoolPreference = memoryPool;
	desc.Properties.CreationNodeMask = 0;
	desc.Properties.VisibleNodeMask = 0;
	desc.Alignment = alignInBytes;
	desc.Flags = flags;

	ComPtr<ID3D12Heap> pHeap;
	DX::ThrowIfFailed(m_d3dDevice->CreateHeap(&desc, IID_GRAPHICS_PPV_ARGS(pHeap.GetAddressOf())));
	return pHeap;
}

ComPtr<ID3D12Resource> DX12Device::CreatePlacedResource(ID3D12Heap * pHeap,
	uint64_t heapOffsetInBytes,
	const D3D12_RESOURCE_DESC * pDesc,
	D3D12_RESOURCE_STATES initialState,
	const D3D12_CLEAR_VALUE * pOptimizedClearValue)
{
	ComPtr<ID3D12Resource> pResource;
	DX::ThrowIfFailed(m_d3dDevice->CreatePlacedResource(pHeap, heapOffsetInBytes, pDesc, initialState, pOptimizedClearValue, IID_GRAPHICS_PPV_ARGS(pResource.GetAddressOf())));
	return pResource;
}

ComPtr<ID3D12Resource> DX12Device::CreateCommittedResource(const D3D12_HEAP_PROPERTIES * pHeapProperties,
	D3D12_HEAP_FLAGS heapFlags,
	const D3D12_RESOURCE_DESC * pResourceDesc,
	D3D12_RESOURCE_STATES initialState,
	const D3D12_CLEAR_VALUE * pOptimizedClearValue)
{
	ComPtr<ID3D12Resource> pResource;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(pHeapProperties,
		heapFlags,
		pResourceDesc,
		initialState,
		pOptimizedClearValue,
		IID_GRAPHICS_PPV_ARGS(pResource.GetAddressOf())));
	return pResource;
}

ComPtr<ID3D12Resource> DX12Device::CreateCommittedBufferInDefaultHeap(uint64_t sizeInBytes, uint64_t alignInBytes, D3D12_RESOURCE_STATES initialState)
{
	ComPtr<ID3D12Resource> pResource;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeInBytes, alignInBytes }),
		initialState,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(pResource.GetAddressOf())));
	return pResource;
}

ComPtr<ID3D12Resource> DX12Device::CreateCommittedTexture2DInDefaultHeap(DXGI_FORMAT format,
	uint32_t width,
	uint32_t height,
	uint32_t arraySize,
	uint32_t mipLevels,
	uint32_t sampleCount,
	uint32_t sampleQuality,
	D3D12_RESOURCE_FLAGS flags,
	D3D12_TEXTURE_LAYOUT layout,
	const D3D12_CLEAR_VALUE* pOptimizedClearValue,
	D3D12_RESOURCE_STATES initialState)
{
	ComPtr<ID3D12Resource> pResource;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, layout),
		initialState,
		pOptimizedClearValue,
		IID_GRAPHICS_PPV_ARGS(pResource.GetAddressOf())));
	return pResource;
}

ComPtr<ID3D12DescriptorHeap> DX12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC * desc)
{
	ComPtr<ID3D12DescriptorHeap> pHeap;
    DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(desc, IID_GRAPHICS_PPV_ARGS(pHeap.GetAddressOf())));
	return pHeap;
}

uint32_t DX12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType)
{
	return m_d3dDevice->GetDescriptorHandleIncrementSize(descriptorHeapType);
}

ComPtr<ID3D12RootSignature> DX12Device::CreateRootSignature(const void * pBlobWithRootSignature, size_t blobLengthInBytes)
{
	ComPtr<ID3D12RootSignature> pRootSig;
    DX::ThrowIfFailed(m_d3dDevice->CreateRootSignature(0, pBlobWithRootSignature, blobLengthInBytes, IID_GRAPHICS_PPV_ARGS(pRootSig.GetAddressOf())));
	return pRootSig;	
}

ComPtr<ID3D12PipelineState> DX12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC * desc)
{
	ComPtr<ID3D12PipelineState> pPSO;
    DX::ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(desc, IID_GRAPHICS_PPV_ARGS(pPSO.GetAddressOf())));
	return pPSO;	
}

#ifdef _XBOX_ONE
ComPtr<IDXGISwapChain1> DX12Device::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_HWND hwnd)
{
	ComPtr<IDXGISwapChain1> pSwapChain;

	// First, retrieve the underlying DXGI device from the D3D device.
	ComPtr<IDXGIDevice1> dxgiDevice;
	DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

	// Identify the physical adapter (GPU or card) this device is running on.
	ComPtr<IDXGIAdapter> dxgiAdapter;
	DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

	// And obtain the factory object that created it.
	ComPtr<IDXGIFactory2> dxgiFactory;
	DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_GRAPHICS_PPV_ARGS(dxgiFactory.GetAddressOf())));

	// Create a swap chain for the window.
	DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
		m_d3dDevice.Get(),
		hwnd,
		swapChainDesc,
		nullptr,
		pSwapChain.GetAddressOf()
	));

	return pSwapChain;
}
#else
ComPtr<IDXGISwapChain1> DX12Device::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_HWND hwnd, ID3D12CommandQueue* pQueue)
{
	ComPtr<IDXGISwapChain1> pSwapChain;

	ComPtr<IDXGIFactory4> dxgiFactory;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_GRAPHICS_PPV_ARGS(&dxgiFactory)));

	DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
		pQueue,		// Swap chain needs the queue so that it can force a flush on it.
		hwnd,
		swapChainDesc,
		nullptr,
		nullptr,
		pSwapChain.GetAddressOf()
	));

	return pSwapChain;
}
#endif

ComPtr<ID3D12Fence> DX12Device::CreateFence(uint64_t initialValue)
{
	ComPtr<ID3D12Fence> pFence;
    DX::ThrowIfFailed(m_d3dDevice->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(pFence.GetAddressOf())));
	return pFence;
}

void DX12Device::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_d3dDevice->CreateShaderResourceView(pResource, pDesc, DestDescriptor);
}

void DX12Device::CreateRenderTargetView(ID3D12Resource * pResource, const D3D12_RENDER_TARGET_VIEW_DESC * pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_d3dDevice->CreateRenderTargetView(pResource, pDesc, DestDescriptor);
}

void DX12Device::CreateDepthStencilView(ID3D12Resource * pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC * pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor)
{
    m_d3dDevice->CreateDepthStencilView(pResource, pDesc, DestDescriptor);
}

void DX12Device::CopyDescriptorsSimple(uint32_t numDescriptors, D3D12_CPU_DESCRIPTOR_HANDLE destDescriptorRangeStart, D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptorRangeStart, D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapsType)
{
	m_d3dDevice->CopyDescriptorsSimple(numDescriptors, destDescriptorRangeStart, srcDescriptorRangeStart, descriptorHeapsType);
}
