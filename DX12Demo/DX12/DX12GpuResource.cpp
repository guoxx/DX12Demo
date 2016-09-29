#include "pch.h"
#include "DX12GpuResource.h"


DX12GpuResource::DX12GpuResource()
{
}

DX12GpuResource::DX12GpuResource(ComPtr<ID3D12Resource> resource)
	: m_Resource{ resource }
{
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
	m_Resource->Unmap(subresource);
}
