#include "pch.h"
#include "DX12GraphicManager.h"

#include "DX12Device.h"
#include "DX12DescriptorManager.h"
#include "DX12GraphicContext.h"
#include "DX12Fence.h"
#include "DX12GpuResource.h"
#include "DX12Texture.h"

DX12GraphicManager* DX12GraphicManager::s_GfxManager = nullptr;

void DX12GraphicManager::Initialize()
{
	s_GfxManager = new DX12GraphicManager();
}

void DX12GraphicManager::Finalize()
{
	delete s_GfxManager;
	s_GfxManager = nullptr;
}

DX12GraphicManager::DX12GraphicManager()
	: m_UploadHeapAllocator{ 0, DX12UploadHeapSizeInBytes }
{
#if defined(_DEBUG)
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

	m_GraphicContextIdx = 0;
	for (uint32_t i = 0; i < m_GraphicContexts.size(); ++i)
	{
		m_GraphicContexts[i] = std::make_shared<DX12GraphicContext>(m_Device.get());
	}

	m_UploadHeap = m_Device->CreateHeap(DX12UploadHeapSizeInBytes,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

	m_SwapChainCommandQueue = m_Device->CreateGraphicCommandQueue(0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0);
}

DX12GraphicManager::~DX12GraphicManager()
{
}

#ifdef _XBOX_ONE
void DX12GraphicManager::Suspend()
{
	for (auto queue : m_GraphicQueues)
	{
		queue->SuspendX(0);
	}
}

void DX12GraphicManager::Resume()
{
	for (auto queue : m_GraphicQueues)
	{
		queue->ResumeX();
	}
}
#endif

void DX12GraphicManager::CreateGraphicCommandQueues(uint32_t cnt)
{
	for (uint32_t i = 0; i < cnt; ++i)
	{
		m_GraphicQueues.push_back(ComPtr<ID3D12CommandQueue>{ m_Device->CreateGraphicCommandQueue(0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0) });
	}
}

DX12GraphicContext* DX12GraphicManager::BegineGraphicContext()
{
	uint32_t idx = m_GraphicContextIdx % DX12NumGraphicContexts;
	++m_GraphicContextIdx;

	std::shared_ptr<DX12GraphicContext> ctx = m_GraphicContexts[idx];
	ctx->Reset();
	return ctx.get();
}

void DX12GraphicManager::EndGraphicContext(DX12GraphicContext * ctx)
{
	ctx->Close();
}

void DX12GraphicManager::ExecuteGraphicContext(DX12GraphicContext* ctx)
{
	ctx->ExecuteInQueue(m_GraphicQueues[0].Get());
}

DX12DescriptorHandle DX12GraphicManager::RegisterResourceInDescriptorHeap(ID3D12Resource * resource, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return m_DescriptorManager->AllocateInHeap(type);
}

void DX12GraphicManager::UpdateBufer(DX12GraphicContext* pGfxContext, DX12GpuResource* pResource, void * pSrcData, uint64_t sizeInBytes)
{
	uint64_t heapOffset = m_UploadHeapAllocator.Alloc(sizeInBytes, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	ComPtr<ID3D12Resource> uploadResource = m_Device->CreatePlacedResource(m_UploadHeap.Get(), heapOffset, &pResource->GetGpuResource()->GetDesc(), D3D12_RESOURCE_STATE_GENERIC_READ);

	// keep a reference to that resource to avoid it been released
	m_TempResources.push_back(uploadResource);

	DX12GpuResource srcResource{ uploadResource };

	void* pUploadData = nullptr;
	srcResource.MapResource(0, &pUploadData);
	std::memcpy(pUploadData, pSrcData, sizeInBytes);
	srcResource.UnmapResource(0);

	pGfxContext->CopyResource(&srcResource, pResource);
}

void DX12GraphicManager::UpdateTexture(DX12GraphicContext * pGfxContext, DX12Texture * pTexture, uint32_t subresource, void * pSrcData, uint64_t sizeInBytes)
{
	uint32_t FirstSubresource = subresource;
	uint32_t NumSubresources = 1;

	uint64_t uploadBufferSize = GetRequiredIntermediateSize(pTexture->GetGpuResource(), FirstSubresource, NumSubresources);
	assert(uploadBufferSize >= sizeInBytes);

	uint64_t heapOffset = m_UploadHeapAllocator.Alloc(uploadBufferSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
	ComPtr<ID3D12Resource> uploadResource = m_Device->CreatePlacedResource(m_UploadHeap.Get(), heapOffset, &CD3DX12_RESOURCE_DESC::Buffer({uploadBufferSize, 0}), D3D12_RESOURCE_STATE_GENERIC_READ);

	// keep a reference to that resource to avoid it been released
	m_TempResources.push_back(uploadResource);

	DX12GpuResource srcResource{ uploadResource };

	void* pUploadData = nullptr;
	srcResource.MapResource(0, &pUploadData);
	std::memcpy(pUploadData, pSrcData, sizeInBytes);
	srcResource.UnmapResource(0);

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[16];
    UINT NumRows[16];
    UINT64 RowSizesInBytes[16];

	{
		D3D12_RESOURCE_DESC Desc = pTexture->GetGpuResource()->GetDesc();
		UINT64 RequiredSize = 0;

		ID3D12Device* pDevice;
		pTexture->GetGpuResource()->GetDevice(__uuidof(*pDevice), reinterpret_cast<void**>(&pDevice));
		pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, 0, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
		pDevice->Release();
	}

	CD3DX12_TEXTURE_COPY_LOCATION Dst(pTexture->GetGpuResource(), subresource);
	CD3DX12_TEXTURE_COPY_LOCATION Src(uploadResource.Get(), Layouts[0]);
	pGfxContext->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
}

