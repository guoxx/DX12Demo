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

DX12StructuredBuffer::DX12StructuredBuffer(DX12Device * device, uint64_t sizeInBytes, uint64_t alignInBytes, uint64_t strideInBytes, DX12GpuResourceUsage resourceUsage)
	: DX12GpuResource()
{
	bool bGpuWritable = (resourceUsage & DX12GpuResourceUsage_GpuWritable) != 0;
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeInBytes, alignInBytes });
	if (bGpuWritable)
	{
		resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	InitializeResource(device, resourceUsage, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ);

	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::BufferView(DXGI_FORMAT_UNKNOWN,
			0,
			static_cast<uint32_t>(sizeInBytes / strideInBytes),
			static_cast<uint32_t>(strideInBytes));
		assert(srvDesc.Buffer.NumElements * strideInBytes == sizeInBytes);

		m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_SRV.GetCpuHandle());

		m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_StagingSRV.GetCpuHandle());
	}

	if (bGpuWritable)
	{
		CD3DX12_UNORDERED_ACCESS_VIEW_DESC uavDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::BufferView(DXGI_FORMAT_UNKNOWN,
			0,
			static_cast<uint32_t>(sizeInBytes / strideInBytes),
			static_cast<uint32_t>(strideInBytes));
		assert(uavDesc.Buffer.NumElements * strideInBytes == sizeInBytes);

		m_UAV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateUnorderedAccessView(GetGpuResource(), &uavDesc, m_UAV.GetCpuHandle());

		m_StagingUAV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateUnorderedAccessView(GetGpuResource(), &uavDesc, m_StagingUAV.GetCpuHandle());
	}
}

DX12StructuredBuffer::~DX12StructuredBuffer()
{
}
