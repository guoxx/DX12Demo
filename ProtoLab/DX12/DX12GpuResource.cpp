#include "pch.h"
#include "DX12GpuResource.h"

#include "DX12Device.h"


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

void DX12GpuResource::InitializeResource(DX12Device* device,
	DX12GpuResourceUsage resourceUsage,
	const D3D12_RESOURCE_DESC* pResourceDesc,
	D3D12_RESOURCE_STATES initialState,
	const D3D12_CLEAR_VALUE * pOptimizedClearValue)
{
	CD3DX12_HEAP_PROPERTIES heapProperties;
	D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
	switch (resourceUsage)
	{
	case DX12GpuResourceUsage_GpuReadOnly:
		heapProperties = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT };
		break;
	case DX12GpuResourceUsage_GpuReadWrite:
		heapProperties = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT };
		break;
	case DX12GpuResourceUsage_CpuWrite_GpuRead:
		heapProperties = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_UPLOAD };
		break;
	case DX12GpuResourceUsage_CpuRead_GpuWrite:
		heapProperties = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_READBACK };
		break;
	default:
		break;
	}

	ComPtr<ID3D12Resource> res = device->CreateCommittedResource(&heapProperties, heapFlags, pResourceDesc, initialState, pOptimizedClearValue);
	SetGpuResource(res, initialState, resourceUsage);
}

void DX12GpuResource::SetGpuResource(ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES usageState, DX12GpuResourceUsage resourceUsage)
{
	m_Resource = resource;
	m_UsageState = usageState;
	m_ResourceUsage = resourceUsage;

	for (int32_t i = 0; i < _countof(m_PendingTransitionState); ++i)
	{
		m_PendingTransitionState[i] = D3D12_RESOURCE_STATE_INVALID;
	}
}
