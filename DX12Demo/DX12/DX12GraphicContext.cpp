#include "pch.h"
#include "DX12GraphicContext.h"

#include "DX12Device.h"
#include "DX12GpuResource.h"
#include "DX12Buffer.h"
#include "DX12ColorSurface.h"
#include "DX12DepthSurface.h"
#include "DX12RootSignature.h"
#include "DX12PipelineState.h"
#include "DX12DescriptorHandle.h"
#include "DX12GraphicManager.h"


DX12GraphicContext::DX12GraphicContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_FlushPendingBarriers = false;

	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
	m_BarriersCommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
}

DX12GraphicContext::~DX12GraphicContext()
{
}

void DX12GraphicContext::PIXBeginEvent(const wchar_t* label)
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

void DX12GraphicContext::PIXEndEvent(void)
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

void DX12GraphicContext::PIXSetMarker(const wchar_t* label)
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


void DX12GraphicContext::IASetIndexBuffer(const DX12IndexBuffer* pIndexBuffer)
{
	m_CommandList->IASetIndexBuffer(&pIndexBuffer->GetView());
}

void DX12GraphicContext::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void DX12GraphicContext::ResolvePendingBarriers()
{
	m_FlushPendingBarriers = m_PendingTransitionBarriers.size();

	if (m_FlushPendingBarriers)
	{
		m_BarriersCommandList->Reset(m_CommandAllocator.Get(), nullptr);

		for (auto pendingBarrier : m_PendingTransitionBarriers)
		{
			D3D12_RESOURCE_STATES stateBeforeThisCommandContext = pendingBarrier.m_DX12Resource->GetUsageState();
			D3D12_RESOURCE_STATES stateAfterThisCommandContext = pendingBarrier.m_DX12Resource->GetPendingTransitionState(GetParallelId());
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

		m_BarriersCommandList->Close();

		m_PendingTransitionBarriers.clear();
	}
}

#if 0
void DX12GraphicContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource)
{
	// only support all subresources transition barrier for the moment
	assert(subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
#else
void DX12GraphicContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateAfter)
{
	uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
#endif

	assert(stateAfter != D3D12_RESOURCE_STATE_INVALID);

	D3D12_RESOURCE_STATES usageStateForThisCommandContext = resource->GetPendingTransitionState(GetParallelId());
	resource->SetPendingTransitionState(stateAfter, GetParallelId());

	D3D12_RESOURCE_STATES stateBefore = usageStateForThisCommandContext;
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->m_Resource.Get(), stateBefore, stateAfter, subresource);

	if (usageStateForThisCommandContext != D3D12_RESOURCE_STATE_INVALID)
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

void DX12GraphicContext::SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap** ppDescriptorHeaps)
{
	m_CommandList->SetDescriptorHeaps(numDescriptorHeaps, ppDescriptorHeaps);
}

void DX12GraphicContext::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void * pSrcData, uint32_t destOffsetIn32BitValues)
{
	m_CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
}

void DX12GraphicContext::SetGraphicsRootDynamicConstantBufferView(uint32_t rootParameterIndex, void* pData, uint32_t sizeInBytes)
{
	void* pDestData = nullptr;
	D3D12_GPU_VIRTUAL_ADDRESS GpuVirtualAddress;
	DX12GraphicManager::GetInstance()->AllocateConstantsBuffer(sizeInBytes, 4, &pDestData, &GpuVirtualAddress);
	std::memcpy(pDestData, pData, sizeInBytes);

	m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, GpuVirtualAddress);
}

void DX12GraphicContext::SetGraphicsRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer)
{
	m_CommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, pStructuredBuffer->GetGpuResource()->GetGPUVirtualAddress());
}

void DX12GraphicContext::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle)
{
	m_CommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptorHandle.GetGpuHandle());
}

void DX12GraphicContext::SetGraphicsRootSignature(DX12RootSignature * pRootSig)
{
	m_CommandList->SetGraphicsRootSignature(pRootSig->GetSignature());
}

void DX12GraphicContext::SetPipelineState(DX12PipelineState * pPSO)
{
	m_CommandList->SetPipelineState(pPSO->GetPSO());
}

void DX12GraphicContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	m_CommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

void DX12GraphicContext::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION * pDst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION * pSrc, const D3D12_BOX * pSrcBox)
{
	m_CommandList->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
}

void DX12GraphicContext::CopyResource(DX12GpuResource* srcResource, DX12GpuResource* dstResource)
{
	m_CommandList->CopyResource(dstResource->GetGpuResource(), srcResource->GetGpuResource());
}

void DX12GraphicContext::ClearRenderTarget(DX12ColorSurface * pColorSurface, float r, float g, float b, float a)
{
    DirectX::XMVECTORF32 clearColor;
	clearColor.v = {r, g, b, a};

    m_CommandList->ClearRenderTargetView(pColorSurface->GetRTV().GetCpuHandle(), clearColor, 0, nullptr);

}

void DX12GraphicContext::ClearDepthTarget(DX12DepthSurface* pDepthSurface, float d)
{
    m_CommandList->ClearDepthStencilView(pDepthSurface->GetDSV().GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
}

void DX12GraphicContext::SetRenderTargets(uint32_t numColorSurfaces, DX12ColorSurface * pColorSurface[], DX12DepthSurface * pDepthSurface)
{
	D3D12_CPU_DESCRIPTOR_HANDLE srvs[DX12MaxRenderTargetsCount];
	for (uint32_t i = 0; i < numColorSurfaces; ++i)
	{
		srvs[i] = pColorSurface[i]->GetRTV().GetCpuHandle();
	}
	m_CommandList->OMSetRenderTargets(numColorSurfaces, srvs, false, pDepthSurface != nullptr ? &pDepthSurface->GetDSV().GetCpuHandle() : nullptr);
}

void DX12GraphicContext::SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height, float minDepth, float maxDepth)
{
    D3D12_VIEWPORT viewport = { (float)topLeftX, (float)topLeftY, (float)(width), (float)(height), minDepth, maxDepth };
    m_CommandList->RSSetViewports(1, &viewport);

	// TODO: remove this code
    D3D12_RECT scissorRect = { (float)topLeftX, (float)topLeftY, (float)(topLeftX + width), (float)(topLeftY + height) };
    m_CommandList->RSSetScissorRects(1, &scissorRect);
}

void DX12GraphicContext::ExecuteInQueue(ID3D12CommandQueue* pCommandQueue)
{
	if (m_FlushPendingBarriers)
	{
		m_FlushPendingBarriers = false;

		ID3D12CommandList* lists[] = { m_BarriersCommandList.Get(), m_CommandList.Get() };
		pCommandQueue->ExecuteCommandLists(_countof(lists), lists);

		DX12GraphicManager::GetInstance()->GetFenceManager()->SignalAndAdvance(pCommandQueue);
	}
	else
	{
		super::ExecuteInQueue(pCommandQueue);
	}
}
