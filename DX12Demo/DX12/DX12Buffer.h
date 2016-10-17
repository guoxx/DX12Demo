#pragma once

#include "DX12GpuResource.h"
#include "DX12DescriptorHandle.h"

class DX12Device;

class DX12IndexBuffer : public DX12GpuResource
{
public:
	DX12IndexBuffer(DX12Device* device, uint64_t sizeInBytes, uint64_t alignInBytes, DXGI_FORMAT fmt);
	virtual ~DX12IndexBuffer();

	const D3D12_INDEX_BUFFER_VIEW& GetView() const { return m_View; };

private:
	DXGI_FORMAT m_Format;
	D3D12_INDEX_BUFFER_VIEW m_View;
};

class DX12StructuredBuffer : public DX12GpuResource
{
public:
	DX12StructuredBuffer(DX12Device* device, uint64_t sizeInBytes, uint64_t alignInBytes, uint64_t strideInBytes);
	virtual ~DX12StructuredBuffer();

	const DX12DescriptorHandle& GetDescriptorHandle() const { return m_DescriptorHandle; }

private:
	DX12DescriptorHandle m_DescriptorHandle;
};