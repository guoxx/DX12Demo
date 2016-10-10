#pragma once

#include "DX12DescriptorHandle.h"

class DX12Device;
class DX12GraphicContext;

class DX12DescriptorManager
{
public:
	DX12DescriptorManager(DX12Device* device);
	~DX12DescriptorManager();

	DX12DescriptorHandle AllocateInHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType);

	void SetupHeapsForCommandList(DX12GraphicContext* pGfxContext);

private:
	enum
	{
		DefaultDescriptorHeapSize = 2048,
	};

	std::array<ComPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHeaps;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHeapOffset;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHandleIncrementSizeForHeaps;
};

