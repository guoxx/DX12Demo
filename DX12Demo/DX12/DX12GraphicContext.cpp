#include "pch.h"
#include "DX12GraphicContext.h"

#include "DX12Device.h"
#include "DX12GpuResource.h"

DX12GraphicContext::DX12GraphicContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
}

DX12GraphicContext::~DX12GraphicContext()
{
}

void DX12GraphicContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource)
{
    // Transition the render target into the correct state to allow for drawing into it.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->m_Resource.Get(), stateAfter, stateAfter, subresource);
    m_CommandList->ResourceBarrier(1, &barrier);
}
