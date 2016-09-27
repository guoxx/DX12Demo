#pragma once

class DX12DescriptorManager;

class DX12DescriptorHandle
{
	friend class DX12DescriptorManager;

public:
	DX12DescriptorHandle() = default;
	~DX12DescriptorHandle() = default;

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() const { return m_CpuHandle; }	
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() const { return m_GpuHandle; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_CpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE m_GpuHandle;
};