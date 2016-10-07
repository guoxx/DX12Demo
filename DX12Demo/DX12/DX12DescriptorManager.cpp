#include "pch.h"
#include "DX12DescriptorManager.h"

#include "DX12Device.h"

DX12DescriptorManager::DX12DescriptorManager(DX12Device * device)
{
	for (uint32_t i = 0; i < m_DescriptorHeaps.size(); ++i)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
		bool isShaderVisible = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) || (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = DefaultDescriptorHeapSize;
		desc.Type = type;
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		m_DescriptorHeaps[i] = ComPtr<ID3D12DescriptorHeap>{ device->CreateDescriptorHeap(&desc) };
		m_DescriptorHeapOffset[i] = 0;
		m_DescriptorHandleIncrementSizeForHeaps[i] = device->GetDescriptorHandleIncrementSize(type);
	}
}

DX12DescriptorManager::~DX12DescriptorManager()
{
}

DX12DescriptorHandle DX12DescriptorManager::AllocateInHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	assert(m_DescriptorHeapOffset[heapType] < DefaultDescriptorHeapSize);

	uint32_t offset = m_DescriptorHeapOffset[heapType] * m_DescriptorHandleIncrementSizeForHeaps[heapType];
	m_DescriptorHeapOffset[heapType] += 1;

	DX12DescriptorHandle handle;
	handle.m_CpuHandle = m_DescriptorHeaps[heapType]->GetCPUDescriptorHandleForHeapStart();
	handle.m_GpuHandle = m_DescriptorHeaps[heapType]->GetGPUDescriptorHandleForHeapStart();
	handle.m_CpuHandle.ptr += offset;
	handle.m_GpuHandle.ptr += offset;

	return handle;
}
