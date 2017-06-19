#include "pch.h"
#include "DX12GraphicsManager.h"

#include "DX12Device.h"
#include "DX12DescriptorManager.h"
#include "DX12GraphicsContext.h"
#include "DX12Fence.h"
#include "DX12GpuResource.h"
#include "DX12Texture.h"

DX12GraphicsManager* DX12GraphicsManager::s_GfxManager = nullptr;

void DX12GraphicsManager::Initialize()
{
	s_GfxManager = new DX12GraphicsManager();
}

void DX12GraphicsManager::Finalize()
{
	delete s_GfxManager;
	s_GfxManager = nullptr;
}

DX12GraphicsManager::DX12GraphicsManager()
	: m_UploadHeapAllocator{ 0, DX12UploadHeapSizeInBytes }
	, m_ConstantsBufferAllocator{ 0, DX12ConstantsBufferHeapSizeInBytes }
	, m_TempResourcePoolIdx{ 0 }
{
#ifndef NDEBUG
    // Enable the D3D12 debug layer.
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_GRAPHICS_PPV_ARGS(debugController.GetAddressOf()))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

	m_Device = std::make_unique<DX12Device>();

	m_DescriptorManager = std::make_unique<DX12DescriptorManager>(m_Device.get());

	m_FenceManager = std::make_unique<DX12FenceManager>(m_Device.get());

	m_GraphicContextParallelBits = 0x00;

	m_GraphicContextIdx = 0;
	for (uint32_t i = 0; i < m_GraphicContexts.size(); ++i)
	{
		m_GraphicContexts[i] = std::make_shared<DX12GraphicsContext>(m_Device.get());
	}

	m_UploadHeap = m_Device->CreateHeap(DX12UploadHeapSizeInBytes,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

	m_ConstantsBufferHeap = m_Device->CreateHeap(DX12ConstantsBufferHeapSizeInBytes,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

	m_ConstantsBuffer = m_Device->CreatePlacedResource(m_ConstantsBufferHeap.Get(), 0, &CD3DX12_RESOURCE_DESC::Buffer(DX12ConstantsBufferHeapSizeInBytes), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
	m_ConstantsBuffer->Map(0, nullptr, (void**)&m_ConstantsBufferBeginPtr);

#ifndef _XBOX_ONE
	m_SwapChainCommandQueue = m_Device->CreateGraphicCommandQueue(0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0);
#endif
}

DX12GraphicsManager::~DX12GraphicsManager()
{
}

void DX12GraphicsManager::Flip()
{
	m_TempResourcePoolIdx = (m_TempResourcePoolIdx + 1) % NumTempResourcesPool;
	m_TempResources[m_TempResourcePoolIdx].clear();
}

#ifdef _XBOX_ONE
void DX12GraphicsManager::Suspend()
{
	for (auto queue : m_GraphicQueues)
	{
		queue->SuspendX(0);
	}
}

void DX12GraphicsManager::Resume()
{
	for (auto queue : m_GraphicQueues)
	{
		queue->ResumeX();
	}
}
#endif

void DX12GraphicsManager::CreateGraphicCommandQueues(uint32_t cnt)
{
	for (uint32_t i = 0; i < cnt; ++i)
	{
		m_GraphicQueues.push_back(ComPtr<ID3D12CommandQueue>{ m_Device->CreateGraphicCommandQueue(0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) });
	}

#ifdef _XBOX_ONE
	m_SwapChainCommandQueue = m_GraphicQueues[0];
#endif
}

DX12GraphicsContext* DX12GraphicsManager::BegineGraphicsContext()
{
	int32_t parallelId = DX12ParallelIdInvalid;
	for (int32_t i = 0; i < DX12MaxGraphicContextsInParallel; ++i)
	{
		if ((m_GraphicContextParallelBits & (0x01 << i)) == 0)
		{
			parallelId = i;
			m_GraphicContextParallelBits |= (0x01 << i);
			break;
		}
	}
	assert(parallelId != DX12ParallelIdInvalid);

	uint32_t idx = m_GraphicContextIdx % DX12NumGraphicContexts;
	++m_GraphicContextIdx;

	std::shared_ptr<DX12GraphicsContext> ctx = m_GraphicContexts[idx];
	ctx->Reset();
	ctx->SetParallelId(parallelId);

	m_DescriptorManager->SetupHeapsForCommandList(ctx.get());

	return ctx.get();
}

void DX12GraphicsManager::EndGraphicsContext(DX12GraphicsContext * ctx)
{
	ctx->Close();

	ctx->ResolvePendingBarriers();

	int32_t parallelId = ctx->GetParallelId();
	assert(parallelId >= 0 && parallelId < DX12MaxGraphicContextsInParallel);
	assert((m_GraphicContextParallelBits & (0x01 << parallelId)) != 0);
	m_GraphicContextParallelBits &= ~(0x01 << parallelId);
	ctx->SetParallelId(DX12ParallelIdInvalid);
}

void DX12GraphicsManager::ExecuteGraphicsContext(DX12GraphicsContext* ctx)
{
	ExecuteGraphicsContextInQueue(ctx, m_GraphicQueues[0].Get());
}

void DX12GraphicsManager::ExecuteGraphicsContextInQueue(DX12GraphicsContext* ctx, ID3D12CommandQueue* pQueue)
{
	PIXBeginEvent(0, L"ExecuteCommandList");
	ctx->ExecuteInQueue(pQueue);
	PIXEndEvent();
}

DX12DescriptorHandle DX12GraphicsManager::RegisterResourceInDescriptorHeap(ID3D12Resource * resource, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return m_DescriptorManager->AllocateInHeap(type);
}

DX12DescriptorHandle DX12GraphicsManager::RegisterResourceInStagingDescriptorHeap(ID3D12Resource * resource, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return m_DescriptorManager->AllocateInStagingHeap(type);
}

std::shared_ptr<DX12GpuResource> DX12GraphicsManager::AllocateTempGpuResourceInUploadHeap(uint32_t sizeInBytes, uint32_t alignInBytes)
{
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ;
	uint64_t heapOffset = m_UploadHeapAllocator.Alloc(sizeInBytes, alignInBytes);
	ComPtr<ID3D12Resource> uploadResource = m_Device->CreatePlacedResource(m_UploadHeap.Get(), heapOffset, &CD3DX12_RESOURCE_DESC::Buffer({sizeInBytes, 0}), initialState);
	std::shared_ptr<DX12GpuResource> tempGpuResource = std::make_shared<DX12GpuResource>(uploadResource, initialState);

	// keep a reference to that resource to avoid it been released
	m_TempResources[m_TempResourcePoolIdx].push_back(tempGpuResource);

	return tempGpuResource;
}

void DX12GraphicsManager::AllocateConstantsBuffer(uint32_t sizeInBytes, uint32_t alignInBytes, void** pCpuWrittablePtr, D3D12_GPU_VIRTUAL_ADDRESS* pGpuVirtualAddress)
{
	//assert(alignInBytes == 0);
	uint64_t offset = m_ConstantsBufferAllocator.Alloc(sizeInBytes, alignInBytes);
	*pCpuWrittablePtr = (void*)((uint64_t)m_ConstantsBufferBeginPtr + offset);
	*pGpuVirtualAddress = m_ConstantsBuffer->GetGPUVirtualAddress() + offset;
}

