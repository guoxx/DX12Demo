#include "pch.h"
#include "DX12Buffer.h"

#include "DX12Wrapper.h"
#include "DX12Device.h"
#include "DX12GraphicManager.h"


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

	CD3DX12_SHADER_RESOURCE_VIEW_DESC desc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::BufferView(DXGI_FORMAT_UNKNOWN,
		0,
		static_cast<uint32_t>(sizeInBytes / strideInBytes),
		static_cast<uint32_t>(strideInBytes));
	assert(desc.Buffer.NumElements * strideInBytes == sizeInBytes);

	m_DescriptorHandle = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &desc, m_DescriptorHandle.GetCpuHandle());
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
}
