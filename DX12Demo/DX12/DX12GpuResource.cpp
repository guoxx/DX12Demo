#include "pch.h"
#include "DX12GpuResource.h"


DX12GpuResource::DX12GpuResource()
{
	SetGpuResource(ComPtr<ID3D12Resource>{ nullptr }, D3D12_RESOURCE_STATE_INVALID);
}

DX12GpuResource::DX12GpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState)
{
	SetGpuResource(resource, usageState);
}

DX12GpuResource::~DX12GpuResource()
{
}

void DX12GpuResource::MapResource(uint32_t subresource, void** ppData)
{
	m_Resource->Map(subresource, nullptr, ppData);
}

void DX12GpuResource::UnmapResource(uint32_t subresource)
{
	m_Resource->Unmap(subresource, nullptr);
}

void DX12GpuResource::SetGpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState)
{
	m_Resource = resource;
	m_UsageState = usageState;

	for (int32_t i = 0; i < _countof(m_PendingTransitionState); ++i)
	{
		m_PendingTransitionState[i] = D3D12_RESOURCE_STATE_INVALID;
	}
}
