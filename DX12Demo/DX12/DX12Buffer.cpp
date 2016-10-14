#include "pch.h"
#include "DX12Buffer.h"

#include "DX12Device.h"
#include "DX12GraphicManager.h"


DX12ConstantsBuffer::DX12ConstantsBuffer(DX12Device * device, uint64_t sizeInBytes, uint64_t alignInBytes)
{
	SetGpuResource(device->CreateCommittedBufferInDefaultHeap(sizeInBytes, alignInBytes, D3D12_RESOURCE_STATE_GENERIC_READ), D3D12_RESOURCE_STATE_GENERIC_READ);
}

DX12ConstantsBuffer::~DX12ConstantsBuffer()
{
}

DX12IndexBuffer::DX12IndexBuffer(DX12Device* device, uint64_t sizeInBytes, uint64_t alignInBytes, DXGI_FORMAT fmt)
	: DX12GpuResource()
	, m_Format{ fmt }
	, m_View{}
{
	SetGpuResource(device->CreateCommittedBufferInDefaultHeap(sizeInBytes, alignInBytes, D3D12_RESOURCE_STATE_COMMON), D3D12_RESOURCE_STATE_COMMON);
	m_View = D3D12_INDEX_BUFFER_VIEW{ GetGpuResource()->GetGPUVirtualAddress(), static_cast<uint32_t>(sizeInBytes), m_Format };
}

DX12IndexBuffer::~DX12IndexBuffer()
{
}

DX12StructuredBuffer::DX12StructuredBuffer(DX12Device * device, uint64_t sizeInBytes, uint64_t alignInBytes, uint64_t strideInBytes)
	: DX12GpuResource()
{
	SetGpuResource(device->CreateCommittedBufferInDefaultHeap(sizeInBytes, alignInBytes, D3D12_RESOURCE_STATE_COMMON), D3D12_RESOURCE_STATE_COMMON);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = static_cast<uint32_t>(sizeInBytes / strideInBytes);
	assert(desc.Buffer.NumElements * strideInBytes == sizeInBytes);
	desc.Buffer.StructureByteStride = static_cast<uint32_t>(strideInBytes);
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	m_DescriptorHandle = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &desc, m_DescriptorHandle.GetCpuHandle());
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
}
