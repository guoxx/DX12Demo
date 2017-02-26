#pragma once

#include "DX12Constants.h"


class DX12Device;
class DX12GraphicsContext;

class DX12GpuResource
{
	friend class DX12GraphicsContext;

public:
	DX12GpuResource();
	DX12GpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState);
	virtual ~DX12GpuResource();

	ID3D12Resource* GetGpuResource() const { return m_Resource.Get(); }

	void MapResource(uint32_t subresource, void** ppData);

	void UnmapResource(uint32_t subresource);

	D3D12_RESOURCE_STATES GetUsageState() const { return m_UsageState; }

	void SetUsageState(D3D12_RESOURCE_STATES usageState) { m_UsageState = usageState; }

	D3D12_RESOURCE_STATES GetPendingTransitionState(int32_t parallelId) const
	{
		assert(parallelId != DX12ParallelIdInvalid);
		return m_PendingTransitionState[parallelId];
	}

	void SetPendingTransitionState(D3D12_RESOURCE_STATES pendingState, int32_t parallelId)
	{
		assert(parallelId != DX12ParallelIdInvalid);
		m_PendingTransitionState[parallelId] = pendingState;
	}	

protected:

	void InitializeResource(DX12Device* device,
		DX12GpuResourceUsage resourceUsage,
		const D3D12_RESOURCE_DESC* pResourceDesc,
		D3D12_RESOURCE_STATES initialState,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr);

	void SetGpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState, DX12GpuResourceUsage resourceUsage = DX12GpuResourceUsage_GpuReadOnly);

private:
	ComPtr<ID3D12Resource> m_Resource;

	DX12GpuResourceUsage m_ResourceUsage;
	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_PendingTransitionState[DX12MaxGraphicContextsInParallel];
};
