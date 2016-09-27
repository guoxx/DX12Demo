#include "pch.h"
#include "DX12Buffer.h"

#include "DX12Device.h"
#include "DX12GraphicManager.h"

DX12ConstantsBuffer::DX12ConstantsBuffer(DX12Device * device)
{
}

DX12ConstantsBuffer::~DX12ConstantsBuffer()
{
}

DX12IndexBuffer::DX12IndexBuffer(DX12Device* device, uint32_t sizeInBytes, uint32_t alignInBytes, DXGI_FORMAT fmt)
	: DX12GpuResource()
	, m_Format{ fmt }
	, m_View{}
{
	m_Resource = device->CreateCommittedResourceInDefaultHeap(sizeInBytes, alignInBytes, D3D12_RESOURCE_STATE_COMMON);
	m_View = D3D12_INDEX_BUFFER_VIEW{ m_Resource->GetGPUVirtualAddress(), sizeInBytes, m_Format };
}

DX12IndexBuffer::~DX12IndexBuffer()
{
}

DX12StructuredBuffer::DX12StructuredBuffer(DX12Device * device, uint32_t sizeInBytes, uint32_t alignInBytes, uint32_t strideInBytes)
	: DX12GpuResource()
{
	m_Resource = device->CreateCommittedResourceInDefaultHeap(sizeInBytes, alignInBytes, D3D12_RESOURCE_STATE_COMMON);

	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = sizeInBytes / strideInBytes;
	assert(desc.Buffer.NumElements * strideInBytes == sizeInBytes);
	desc.Buffer.StructureByteStride = strideInBytes;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	m_DescriptorHandle = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_Resource.Get(), &desc, m_DescriptorHandle.GetCpuHandle());
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
}
