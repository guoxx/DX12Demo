#pragma once

#include "DX12CommandContext.h"

class DX12GpuResource;

class DX12GraphicContext : public DX12CommandContext
{
	using super = DX12CommandContext;
public:
	DX12GraphicContext(DX12Device* device);
	virtual ~DX12GraphicContext();

	void ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
};
