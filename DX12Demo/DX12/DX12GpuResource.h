#pragma once

class DX12GpuResource
{
public:
	DX12GpuResource();
	virtual ~DX12GpuResource();

protected:
	ComPtr<ID3D12Resource> m_Resource;
};
