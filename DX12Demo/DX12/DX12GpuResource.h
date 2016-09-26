#pragma once

class DX12GraphicContext;

class DX12GpuResource
{
	friend class DX12GraphicContext;

public:
	DX12GpuResource();
	virtual ~DX12GpuResource();

protected:
	ComPtr<ID3D12Resource> m_Resource;
};
