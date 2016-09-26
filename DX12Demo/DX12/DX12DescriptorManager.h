#pragma once

class DX12Device;

class DX12DescriptorManager
{
public:
	DX12DescriptorManager(DX12Device* device);
	~DX12DescriptorManager();

private:
	enum
	{
		DefaultDescriptorHeapSize = 2048,
	};

	std::array<ComPtr<ID3D12DescriptorHeap>, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHeaps;
	std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> m_DescriptorHandleIncrementSizeForHeaps;
};

