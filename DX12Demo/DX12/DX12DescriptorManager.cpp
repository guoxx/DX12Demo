#include "pch.h"
#include "DX12DescriptorManager.h"

#include "DX12Device.h"
#include "DX12GraphicContext.h"

#ifdef _XBOX_ONE
int32_t DefaultStaticDescriptorHeapSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { 2048 * 4, 1024, 1024, 1024, 1024 };
int32_t DefaultDynamicDescriptorHeapSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { 2048 * 8, 1024, 1024, 1024, 1024 };
#else
int32_t DefaultStaticDescriptorHeapSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { 2048 * 4, 1024, 1024, 1024 };
int32_t DefaultDynamicDescriptorHeapSize[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = { 2048 * 8, 1024, 1024, 1024 };
#endif

DX12DescriptorManager::DX12DescriptorManager(DX12Device * device)
{
	for (uint32_t i = 0; i < m_StagingDescriptorHeaps.size(); ++i)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = DefaultStagingDescriptorHeapSize;
		desc.Type = type;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		m_StagingDescriptorHeaps[i] = ComPtr<ID3D12DescriptorHeap>{ device->CreateDescriptorHeap(&desc) };
		m_StagingDescriptorHeapOffset[i] = 0;
	}

	for (uint32_t i = 0; i < m_DescriptorHeaps.size(); ++i)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;
		bool isShaderVisible = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) || (type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = DefaultStaticDescriptorHeapSize[i] + DefaultDynamicDescriptorHeapSize[i];
		desc.Type = type;
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		m_DescriptorHeaps[i] = ComPtr<ID3D12DescriptorHeap>{ device->CreateDescriptorHeap(&desc) };
		m_StaticDescriptorHeapOffset[i] = 0;
		m_DynamicDescriptorHeapOffset[i] = 0;
		m_DescriptorHandleIncrementSizeForHeaps[i] = device->GetDescriptorHandleIncrementSize(type);
	}
}

DX12DescriptorManager::~DX12DescriptorManager()
{
}

uint32_t DX12DescriptorManager::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	return m_DescriptorHandleIncrementSizeForHeaps[heapType];
}

DX12DescriptorHandle DX12DescriptorManager::AllocateInHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	assert(m_StaticDescriptorHeapOffset[heapType] < DefaultStaticDescriptorHeapSize[heapType]);

	uint32_t offset = m_StaticDescriptorHeapOffset[heapType] * m_DescriptorHandleIncrementSizeForHeaps[heapType];
	m_StaticDescriptorHeapOffset[heapType] += 1;

	DX12DescriptorHandle handle;
	handle.m_CpuHandle = m_DescriptorHeaps[heapType]->GetCPUDescriptorHandleForHeapStart();
	handle.m_GpuHandle = m_DescriptorHeaps[heapType]->GetGPUDescriptorHandleForHeapStart();
	handle.m_CpuHandle.ptr += offset;
	handle.m_GpuHandle.ptr += offset;

	return handle;
}

DX12DescriptorHandle DX12DescriptorManager::AllocateInDynamicHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int32_t numHandles)
{
	if (m_DynamicDescriptorHeapOffset[heapType] + numHandles > DefaultDynamicDescriptorHeapSize[heapType])
	{
		// reuse from begin
		m_DynamicDescriptorHeapOffset[heapType] = 0;
	}

	uint32_t offset = (m_DynamicDescriptorHeapOffset[heapType] + DefaultStaticDescriptorHeapSize[heapType]) * m_DescriptorHandleIncrementSizeForHeaps[heapType];
	m_DynamicDescriptorHeapOffset[heapType] += numHandles;

	DX12DescriptorHandle handle;
	handle.m_CpuHandle = m_DescriptorHeaps[heapType]->GetCPUDescriptorHandleForHeapStart();
	handle.m_GpuHandle = m_DescriptorHeaps[heapType]->GetGPUDescriptorHandleForHeapStart();
	handle.m_CpuHandle.ptr += offset;
	handle.m_GpuHandle.ptr += offset;

	return handle;
}

DX12DescriptorHandle DX12DescriptorManager::AllocateInStagingHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
	assert(m_StagingDescriptorHeapOffset[heapType] < DefaultStagingDescriptorHeapSize);

	uint32_t offset = m_StagingDescriptorHeapOffset[heapType] * m_DescriptorHandleIncrementSizeForHeaps[heapType];
	m_StagingDescriptorHeapOffset[heapType] += 1;

	DX12DescriptorHandle handle;
	handle.m_CpuHandle = m_StagingDescriptorHeaps[heapType]->GetCPUDescriptorHandleForHeapStart();
	handle.m_GpuHandle = m_StagingDescriptorHeaps[heapType]->GetGPUDescriptorHandleForHeapStart();
	handle.m_CpuHandle.ptr += offset;
	handle.m_GpuHandle.ptr += offset;

	return handle;
}

void DX12DescriptorManager::SetupHeapsForCommandList(DX12GraphicContext* pGfxContext)
{
	ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Get(), m_DescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Get() };
	pGfxContext->SetDescriptorHeaps(_countof(heaps), heaps);
}
