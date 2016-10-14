#pragma once

#include "DX12Constants.h"


class DX12GraphicContext;

class DX12GpuResource
{
	friend class DX12GraphicContext;

public:
	DX12GpuResource();
	DX12GpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState);
	virtual ~DX12GpuResource();

	ID3D12Resource* GetGpuResource() const { return m_Resource.Get(); }

	void MapResource(uint32_t subresource, void** ppData);

	void UnmapResource(uint32_t subresource);

protected:
	void SetGpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState);

private:
	ComPtr<ID3D12Resource> m_Resource;

	D3D12_RESOURCE_STATES m_UsageState;
	D3D12_RESOURCE_STATES m_PendingTransitionState[DX12MaxGraphicContextsInParallel];
};
