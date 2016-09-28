#include "pch.h"
#include "DX12GraphicContext.h"

#include "DX12Device.h"
#include "DX12GpuResource.h"
#include "DX12RootSignature.h"
#include "DX12PipelineState.h"

DX12GraphicContext::DX12GraphicContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
}

DX12GraphicContext::~DX12GraphicContext()
{
}

void DX12GraphicContext::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW * pView)
{
	m_CommandList->IASetIndexBuffer(pView);
}

void DX12GraphicContext::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void DX12GraphicContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource)
{
    // Transition the render target into the correct state to allow for drawing into it.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->m_Resource.Get(), stateBefore, stateAfter, subresource);
    m_CommandList->ResourceBarrier(1, &barrier);
}

void DX12GraphicContext::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32 num32BitValuesToSet, const void * pSrcData, uint32 destOffsetIn32BitValues)
{
	m_CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
}

void DX12GraphicContext::SetGraphicsRootSignature(DX12RootSignature * pRootSig)
{
	m_CommandList->SetGraphicsRootSignature(pRootSig->GetSignature());
}

void DX12GraphicContext::SetPipelineState(DX12PipelineState * pPSO)
{
	m_CommandList->SetPipelineState(pPSO->GetPSO());
}

void DX12GraphicContext::DrawIndexed(uint32 indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	m_CommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}
