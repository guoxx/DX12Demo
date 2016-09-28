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
