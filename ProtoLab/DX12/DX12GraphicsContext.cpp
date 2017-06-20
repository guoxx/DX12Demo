#include "pch.h"
#include "DX12GraphicsContext.h"

#include "DX12Device.h"
#include "DX12GpuResource.h"
#include "DX12Buffer.h"
#include "DX12ColorSurface.h"
#include "DX12DepthSurface.h"
#include "DX12RootSignature.h"
#include "DX12PipelineState.h"
#include "DX12DescriptorHandle.h"
#include "DX12DescriptorManager.h"
#include "DX12GraphicsManager.h"


DX12GraphicsContext::DX12GraphicsContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_FlushPendingBarriers = false;

	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
	m_BarriersCommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());

	m_DynamicCbvSrvUavDescriptorsTableDirty = 0x00;
}

DX12GraphicsContext::~DX12GraphicsContext()
{
}

void DX12GraphicsContext::PIXBeginEvent(const wchar_t* label)
{
#if defined(RELEASE) || !defined(_XBOX_ONE) && _MSC_VER < 1800
	(label);
#else

#if _XBOX_ONE
	#if D3D12_SDK_VERSION_MINOR == 0
		m_CommandList->PIXBeginEventX(label);
	#else
		::PIXBeginEvent(m_CommandList.Get(), 0, label);
	#endif
#elif _MSC_VER >= 1800
	::PIXBeginEvent(m_CommandList.Get(), 0, label);
#endif

#endif
}

void DX12GraphicsContext::PIXEndEvent(void)
{
#if !defined(RELEASE)
#if _XBOX_ONE
	#if D3D12_SDK_VERSION_MINOR == 0
		m_CommandList->PIXEndEventX();
	#else
		::PIXEndEvent(m_CommandList.Get());
	#endif
#elif _MSC_VER >= 1800
	::PIXEndEvent(m_CommandList.Get());
#endif
#endif
}

void DX12GraphicsContext::PIXSetMarker(const wchar_t* label)
{
#if defined(RELEASE) || !defined(_XBOX_ONE) && _MSC_VER < 1800
	(label);
#else

#if _XBOX_ONE
	#if D3D12_SDK_VERSION_MINOR == 0
		m_CommandList->PIXSetMarkerX(label);
	#else
		::PIXSetMarker(m_CommandList.Get(), 0, label);
	#endif
#elif _MSC_VER >= 1800
	::PIXSetMarker(m_CommandList.Get(), 0, label);
#endif

#endif
}

void DX12GraphicsContext::SetName(const wchar_t* name)
{
    DX::SetName(m_CommandList.Get(), name);
}

void DX12GraphicsContext::IASetIndexBuffer(const DX12IndexBuffer* pIndexBuffer)
{
	m_CommandList->IASetIndexBuffer(&pIndexBuffer->GetView());
}

void DX12GraphicsContext::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void DX12GraphicsContext::ResolvePendingBarriers()
{
	m_FlushPendingBarriers = (m_PendingTransitionBarriers.size() != 0);

	if (m_FlushPendingBarriers)
	{
		m_BarriersCommandList->Reset(m_CommandAllocator.Get(), nullptr);

		for (auto pendingBarrier : m_PendingTransitionBarriers)
		{
			D3D12_RESOURCE_STATES stateBeforeThisCommandContext = pendingBarrier.m_DX12Resource->GetUsageState();
			D3D12_RESOURCE_STATES stateAfterThisCommandContext = pendingBarrier.m_DX12Resource->GetPendingTransitionState(GetParallelId());
			D3D12_RESOURCE_STATES stateAfterThisBarrier = pendingBarrier.m_Barrier.Transition.StateAfter;

			if (stateBeforeThisCommandContext != stateAfterThisBarrier)
			{
				assert(stateBeforeThisCommandContext != D3D12_RESOURCE_STATE_INVALID);
				assert(stateAfterThisCommandContext != D3D12_RESOURCE_STATE_INVALID);

				// correct pending barrier
				pendingBarrier.m_Barrier.Transition.StateBefore = stateBeforeThisCommandContext;

				// update gpu resource state
				pendingBarrier.m_DX12Resource->SetUsageState(stateAfterThisCommandContext);
				pendingBarrier.m_DX12Resource->SetPendingTransitionState(D3D12_RESOURCE_STATE_INVALID, GetParallelId());

				// populate resource barrier command list
				m_BarriersCommandList->ResourceBarrier(1, &pendingBarrier.m_Barrier);
			}
		}

		m_BarriersCommandList->Close();

		m_PendingTransitionBarriers.clear();
	}
}

void DX12GraphicsContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource)
{
	// TODO: only support all subresources transition barrier for the moment
	assert(subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	assert(stateAfter != D3D12_RESOURCE_STATE_INVALID);

	D3D12_RESOURCE_STATES stateBefore = resource->GetPendingTransitionState(GetParallelId());
	if (stateBefore == stateAfter)
	{
		return;
	}

	// update state for this command context
	resource->SetPendingTransitionState(stateAfter, GetParallelId());

	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->m_Resource.Get(), stateBefore, stateAfter, subresource);
	if (stateBefore != D3D12_RESOURCE_STATE_INVALID)
	{
		m_CommandList->ResourceBarrier(1, &barrier);
	}
	else
	{
		// push into a pending list and resolve it when executed in a queue
		PendingResourcBarrier pendingBarrier;
		pendingBarrier.m_DX12Resource = resource;
		pendingBarrier.m_Barrier = barrier;
		m_PendingTransitionBarriers.push_back(pendingBarrier);
	}
}

void DX12GraphicsContext::SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap** ppDescriptorHeaps)
{
	m_CommandList->SetDescriptorHeaps(numDescriptorHeaps, ppDescriptorHeaps);
}

void DX12GraphicsContext::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void * pSrcData, uint32_t destOffsetIn32BitValues)
{
	m_CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
}

void DX12GraphicsContext::SetGraphicsRootDynamicConstantBufferView(uint32_t rootParameterIndex, void* pData, uint32_t sizeInBytes)
{
	void* pDestData = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;
	DX12GraphicsManager::GetInstance()->AllocateConstantsBuffer(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &pDestData, &GpuVirtualAddress);
	std::memcpy(pDestData, pData, sizeInBytes);

	m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, GpuVirtualAddress);
}

void DX12GraphicsContext::SetGraphicsDynamicVertexBuffer(uint32_t slot, void* pData, uint32_t sizeInBytes, uint32_t strideInBytes)
{
	std::shared_ptr<DX12GpuResource> tempGpuResource = DX12GraphicsManager::GetInstance()->AllocateTempGpuResourceInUploadHeap(sizeInBytes);

	void* pDstData = nullptr;
	tempGpuResource->MapResource(0, &pDstData);
	std::memcpy(pDstData, pData, sizeInBytes);
	tempGpuResource->UnmapResource(0);

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = tempGpuResource->GetGpuResource()->GetGPUVirtualAddress();
	vbv.SizeInBytes = sizeInBytes;
	vbv.StrideInBytes = strideInBytes;

	m_CommandList->IASetVertexBuffers(slot, 1, &vbv);
}

void DX12GraphicsContext::SetGraphicsRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer)
{
	m_CommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, pStructuredBuffer->GetGpuResource()->GetGPUVirtualAddress());
}

void DX12GraphicsContext::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle)
{
	m_CommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptorHandle.GetGpuHandle());
}

void DX12GraphicsContext::SetGraphicsDynamicCbvSrvUav(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
	StageDynamicDescriptor(rootParameterIndex, offsetInTable, descriptorHandle);
}

void DX12GraphicsContext::SetComputeRootDynamicConstantBufferView(uint32_t rootParameterIndex, void * pData, uint32_t sizeInBytes)
{
	void* pDestData = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;
	DX12GraphicsManager::GetInstance()->AllocateConstantsBuffer(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, &pDestData, &GpuVirtualAddress);
	std::memcpy(pDestData, pData, sizeInBytes);

	m_CommandList->SetComputeRootConstantBufferView(rootParameterIndex, GpuVirtualAddress);
}

void DX12GraphicsContext::SetComputeRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer)
{
	m_CommandList->SetComputeRootShaderResourceView(rootParameterIndex, pStructuredBuffer->GetGpuResource()->GetGPUVirtualAddress());
}

void DX12GraphicsContext::SetComputeRootRWStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer)
{
	m_CommandList->SetComputeRootUnorderedAccessView(rootParameterIndex, pStructuredBuffer->GetGpuResource()->GetGPUVirtualAddress());
}

void DX12GraphicsContext::SetComputeRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle)
{
	m_CommandList->SetComputeRootDescriptorTable(rootParameterIndex, baseDescriptorHandle.GetGpuHandle());
}

void DX12GraphicsContext::SetComputeDynamicCbvSrvUav(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
	StageDynamicDescriptor(rootParameterIndex, offsetInTable, descriptorHandle);
}

void DX12GraphicsContext::SetGraphicsRootSignature(std::shared_ptr<DX12RootSignature> pRootSig)
{
	RootSignatureChanged(pRootSig);

	m_CommandList->SetGraphicsRootSignature(pRootSig->GetSignature());
}

void DX12GraphicsContext::SetComputeRootSignature(std::shared_ptr<DX12RootSignature> pRootSig)
{
	RootSignatureChanged(pRootSig);

	m_CommandList->SetComputeRootSignature(pRootSig->GetSignature());
}

void DX12GraphicsContext::SetPipelineState(DX12PipelineState * pPSO)
{
	m_CommandList->SetPipelineState(pPSO->GetPSO());
}

void DX12GraphicsContext::Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
	ApplyDynamicDescriptors(true);

	m_CommandList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void DX12GraphicsContext::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
{
	uint32_t dispatchX = (threadCountX + groupSizeX - 1) / groupSizeX;
	uint32_t dispatchY = (threadCountY + groupSizeY - 1) / groupSizeY;
	Dispatch(dispatchX, dispatchY, 1);
}

void DX12GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	ApplyDynamicDescriptors(false);

	m_CommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

void DX12GraphicsContext::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	ApplyDynamicDescriptors(false);

	m_CommandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void DX12GraphicsContext::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION * pDst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION * pSrc, const D3D12_BOX * pSrcBox)
{
	m_CommandList->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
}

void DX12GraphicsContext::UploadBuffer(DX12GpuResource* pResource, void* pSrcData, uint64_t sizeInBytes)
{
	D3D12_SUBRESOURCE_DATA subData;
	subData.pData = pSrcData;
	subData.RowPitch = sizeInBytes;
	subData.SlicePitch = sizeInBytes;
	UploadGpuResource(pResource, 0, 1, &subData);
}

void DX12GraphicsContext::UploadGpuResource(DX12GpuResource* pDstResource, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* pSubData)
{
	uint32_t uploadBufferSize = static_cast<uint32_t>(GetRequiredIntermediateSize(pDstResource->GetGpuResource(), firstSubresource, numSubresources));
	std::shared_ptr<DX12GpuResource> tempGpuResource = DX12GraphicsManager::GetInstance()->AllocateTempGpuResourceInUploadHeap(uploadBufferSize);

	UpdateSubresources(m_CommandList.Get(),
		pDstResource->GetGpuResource(),
		tempGpuResource->GetGpuResource(),
		0,
		firstSubresource,
		numSubresources,
		pSubData);
}

void DX12GraphicsContext::CopyResource(DX12GpuResource* srcResource, DX12GpuResource* dstResource)
{
	m_CommandList->CopyResource(dstResource->GetGpuResource(), srcResource->GetGpuResource());
}

void DX12GraphicsContext::ClearRenderTarget(DX12ColorSurface * pColorSurface, float r, float g, float b, float a)
{
    DirectX::XMVECTORF32 clearColor;
	clearColor.v = {r, g, b, a};

    m_CommandList->ClearRenderTargetView(pColorSurface->GetRTV().GetCpuHandle(), clearColor, 0, nullptr);

}

void DX12GraphicsContext::ClearDepthTarget(DX12DepthSurface* pDepthSurface, float d)
{
    m_CommandList->ClearDepthStencilView(pDepthSurface->GetDSV().GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
}

void DX12GraphicsContext::SetRenderTarget(DX12ColorSurface* pColorSurface)
{
    D3D12_CPU_DESCRIPTOR_HANDLE srvs[1] = {pColorSurface->GetRTV().GetCpuHandle()};
	m_CommandList->OMSetRenderTargets(1, srvs, false, nullptr);
}

void DX12GraphicsContext::SetRenderTargets(uint32_t numColorSurfaces, DX12ColorSurface * pColorSurface[], DX12DepthSurface * pDepthSurface)
{
	D3D12_CPU_DESCRIPTOR_HANDLE srvs[DX12MaxRenderTargetsCount];
	for (uint32_t i = 0; i < numColorSurfaces; ++i)
	{
		srvs[i] = pColorSurface[i]->GetRTV().GetCpuHandle();
	}
	m_CommandList->OMSetRenderTargets(numColorSurfaces, srvs, false, pDepthSurface != nullptr ? &pDepthSurface->GetDSV().GetCpuHandle() : nullptr);
}

void DX12GraphicsContext::SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height, float minDepth, float maxDepth)
{
    D3D12_VIEWPORT viewport = { (float)topLeftX, (float)topLeftY, (float)(width), (float)(height), minDepth, maxDepth };
    m_CommandList->RSSetViewports(1, &viewport);

	// TODO: remove this code
    D3D12_RECT scissorRect = { (float)topLeftX, (float)topLeftY, (float)(topLeftX + width), (float)(topLeftY + height) };
    m_CommandList->RSSetScissorRects(1, &scissorRect);
}

void DX12GraphicsContext::ExecuteInQueue(ID3D12CommandQueue* pCommandQueue)
{
	if (m_FlushPendingBarriers)
	{
		m_FlushPendingBarriers = false;

		ID3D12CommandList* lists[] = { m_BarriersCommandList.Get(), m_CommandList.Get() };
		pCommandQueue->ExecuteCommandLists(_countof(lists), lists);

		DX12GraphicsManager::GetInstance()->GetFenceManager()->SignalAndAdvance(pCommandQueue);
	}
	else
	{
		super::ExecuteInQueue(pCommandQueue);
	}
}

void DX12GraphicsContext::RootSignatureChanged(std::shared_ptr<DX12RootSignature> pRootSig)
{
	m_CurrentRootSig = pRootSig;

	for (int32_t i = 0; i < DX12MaxSlotsPerShader; ++i)
	{
		CpuDescriptorHandlesCache* pCache = &m_DescriptorHandlesCache[i];

		pCache->Clear();
		pCache->m_RootParamIndex = i;
		pCache->m_TableSize = m_CurrentRootSig->GetDescriptorTableSize(i);
		pCache->m_TableStart = 0;
		pCache->m_TableEnd = 0;
		memset(pCache->m_CachedHandles, 0x00, sizeof(pCache->m_CachedHandles));
	}
}

void DX12GraphicsContext::StageDynamicDescriptor(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle)
{
	assert(rootParameterIndex < DX12MaxSlotsPerShader);

	// mark descriptor ad dirty
	m_DynamicCbvSrvUavDescriptorsTableDirty |= ((uint64_t)0x01 << rootParameterIndex);

	// get descriptor table cache entry
	CpuDescriptorHandlesCache* pCache = &m_DescriptorHandlesCache[rootParameterIndex];
	assert(offsetInTable < pCache->m_TableSize);

	uint32_t indexOfCacheTable = offsetInTable / NumCachedHandlesPerNode;
	uint32_t offsetInCacheTable = offsetInTable - indexOfCacheTable * NumCachedHandlesPerNode;

	for (uint i = 1; i <= indexOfCacheTable; ++i)
	{
		if (pCache->m_Next.get() == nullptr)
		{
			// insert a new node if not exist
			pCache->m_Next = std::make_shared<CpuDescriptorHandlesCache>();

			pCache->m_Next->m_RootParamIndex = pCache->m_RootParamIndex;
			pCache->m_Next->m_TableSize = pCache->m_TableSize;

			pCache->m_Next->m_TableStart = i * NumCachedHandlesPerNode;
			pCache->m_Next->m_TableEnd = i * NumCachedHandlesPerNode;

			memset(pCache->m_Next->m_CachedHandles, 0x00, sizeof(pCache->m_Next->m_CachedHandles));
			pCache->m_Next->m_Next = nullptr;
		}

		pCache = pCache->m_Next.get();
	}

	pCache->m_TableEnd = std::max(pCache->m_TableEnd, offsetInTable + 1);
	pCache->m_CachedHandles[offsetInCacheTable] = descriptorHandle;
	assert(pCache->m_TableEnd - pCache->m_TableStart <= NumCachedHandlesPerNode);
}

void DX12GraphicsContext::ApplyDynamicDescriptors(bool bComputeCommand)
{
	for (int32_t i = 0; i < DX12MaxSlotsPerShader; ++i)
	{
		if (m_DynamicCbvSrvUavDescriptorsTableDirty & ((uint64_t)0x01 << i))
		{
			CpuDescriptorHandlesCache* pCache = &m_DescriptorHandlesCache[i];

			uint32_t cacheTableEnd = pCache->GetTableEnd();
			assert(pCache->m_TableSize == DX12DescriptorRangeUnbounded || pCache->m_TableSize == cacheTableEnd);

			uint32_t heapHandleIncrementSize = DX12GraphicsManager::GetInstance()->GetDescriptorManager()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			DX12DescriptorHandle baseDescriptorHandle = DX12GraphicsManager::GetInstance()->GetDescriptorManager()->AllocateInDynamicHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, cacheTableEnd);

			while (true)
			{
				for (uint32_t j = pCache->m_TableStart; j < pCache->m_TableEnd; ++j)
				{
					uint32_t offsetInCacheTable = j - pCache->m_TableStart;
					if (pCache->m_CachedHandles[offsetInCacheTable].ptr != 0)
					{
						D3D12_CPU_DESCRIPTOR_HANDLE destCpuHandle = baseDescriptorHandle.GetCpuHandle();
						destCpuHandle.ptr += j * heapHandleIncrementSize;
						DX12GraphicsManager::GetInstance()->GetDevice()->CopyDescriptorsSimple(1, destCpuHandle, pCache->m_CachedHandles[offsetInCacheTable], D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					}
				}

				if (pCache->m_Next.get() != nullptr)
				{
					pCache = pCache->m_Next.get();
				}
				else
				{
					break;
				}
			}

			if (bComputeCommand)
			{
				SetComputeRootDescriptorTable(pCache->m_RootParamIndex, baseDescriptorHandle);
			}
			else
			{
				SetGraphicsRootDescriptorTable(pCache->m_RootParamIndex, baseDescriptorHandle);
			}
		}
	}

	m_DynamicCbvSrvUavDescriptorsTableDirty = 0x00;
}
