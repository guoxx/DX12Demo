#pragma once

#include "DX12DescriptorHandle.h"

class DX12Device;
class DX12GraphicsContext;

class DX12DescriptorManager
{
public:
	DX12DescriptorManager(DX12Device* device);
	~DX12DescriptorManager();

	uint32_t GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	DX12DescriptorHandle AllocateInHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	DX12DescriptorHandle AllocateInDynamicHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, int32_t numHandles);

	DX12DescriptorHandle AllocateInStagingHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	void SetupHeapsForCommandList(DX12GraphicsContext* pGfxContext);

private:
	enum
	{
		DefaultStagingDescriptorHeapSize = 2048,
	};

	std::array<ComPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_StagingDescriptorHeaps;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_StagingDescriptorHeapOffset;

	std::array<ComPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHeaps;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_StaticDescriptorHeapOffset;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DynamicDescriptorHeapOffset;

	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHandleIncrementSizeForHeaps;
};

