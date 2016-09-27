#include "pch.h"
#include "DX12Device.h"

DX12Device::DX12Device()
{
    DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_GRAPHICS_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())));
}

DX12Device::~DX12Device()
{
}

ID3D12CommandQueue * DX12Device::CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAGS flags, uint32_t nodeMask)
{
    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = priority;
    desc.Flags = flags;
	desc.NodeMask = nodeMask;

	ID3D12CommandQueue* pQueue = nullptr;
    DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&desc, IID_GRAPHICS_PPV_ARGS(&pQueue)));
	return pQueue;
}

ID3D12CommandAllocator * DX12Device::CreateGraphicCommandAllocator()
{
	ID3D12CommandAllocator* pAllocator = nullptr;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(&pAllocator)));
	return pAllocator;
}

ID3D12GraphicsCommandList * DX12Device::CreateGraphicCommandList(ID3D12CommandAllocator* allocator)
{
	ID3D12GraphicsCommandList* pCommandList = nullptr;
    DX::ThrowIfFailed(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_GRAPHICS_PPV_ARGS(&pCommandList)));
    pCommandList->Close();
	return pCommandList;
}

ID3D12Resource * DX12Device::CreateCommittedBufferInDefaultHeap(uint64_t sizeInBytes, uint64_t alignInBytes, D3D12_RESOURCE_STATES initialState)
{
	ID3D12Resource* pResource = nullptr;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeInBytes, alignInBytes }),
		initialState,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(&pResource)));
	return pResource;
}

ID3D12Resource * DX12Device::CreateCommittedTexture2DInDefaultHeap(DXGI_FORMAT format,
	uint32_t width,
	uint32_t height,
	uint32_t arraySize,
	uint32_t mipLevels,
	D3D12_RESOURCE_STATES initialState,
	const D3D12_CLEAR_VALUE* pOptimizedClearValue)
{
	ID3D12Resource* pResource = nullptr;
	DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize, mipLevels),
		initialState,
		pOptimizedClearValue,
		IID_GRAPHICS_PPV_ARGS(&pResource)));
	return pResource;
}

ID3D12DescriptorHeap * DX12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC * desc)
{
	ID3D12DescriptorHeap* pHeap = nullptr;
    DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(desc, IID_GRAPHICS_PPV_ARGS(&pHeap)));
	return pHeap;
}

uint32_t DX12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType)
{
	return uint32_t();
}

ID3D12RootSignature * DX12Device::CreateRootSignature(const void * pBlobWithRootSignature, size_t blobLengthInBytes)
{
	return nullptr;
}

ID3D12PipelineState * DX12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC * desc)
{
	return nullptr;
}

IDXGISwapChain1 * DX12Device::CreateSwapChain(const DXGI_SWAP_CHAIN_DESC1* swapChainDesc, const GFX_WHND hwnd)
{
	IDXGISwapChain1* pSwapChain = nullptr;

#ifdef _XBOX_ONE
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
		&pSwapChain
	));
#else
	ComPtr<IDXGIFactory4> dxgiFactory;
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_GRAPHICS_PPV_ARGS(&dxgiFactory)));

	assert(false);
	//DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
	//	m_d3dDevice.Get(),		// Swap chain needs the queue so that it can force a flush on it.
	//	hwnd,
	//	swapChainDesc,
	//	nullptr,
	//	nullptr,
	//	&pSwapChain
	//));
#endif

	return pSwapChain;
}

ID3D12Fence* DX12Device::CreateFence(uint64_t initialValue)
{
	ID3D12Fence* pFence = nullptr;
    DX::ThrowIfFailed(m_d3dDevice->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(&pFence)));
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
