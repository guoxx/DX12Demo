#include "pch.h"
#include "DX12DescriptorManager.h"

#include "DX12Device.h"

DX12DescriptorManager::DX12DescriptorManager(DX12Device * device)
{
	for (uint32_t i = 0; i < m_DescriptorHeaps.size(); ++i)
	{
		D3D12_DESCRIPTOR_HEAP_TYPE type = (D3D12_DESCRIPTOR_HEAP_TYPE)i;

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = DefaultDescriptorHeapSize;
		desc.Type = type;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		m_DescriptorHeaps[i] = ComPtr<ID3D12DescriptorHeap>{ device->CreateDescriptorHeap(&desc) };
		m_DescriptorHandleIncrementSizeForHeaps[i] = device->GetDescriptorHandleIncrementSize(type);
	}
}

DX12DescriptorManager::~DX12DescriptorManager()
{
}
