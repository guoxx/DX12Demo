#pragma once

class DX12Device;

class DX12Buffer
{
public:
	DX12Buffer(DX12Device* device);
	virtual ~DX12Buffer();

private:
	ComPtr<ID3D12Resource> m_Buffer;
};

class DX12ConstantsBuffer : public DX12Buffer
{
public:
	DX12ConstantsBuffer(DX12Device* device);
	virtual ~DX12ConstantsBuffer();
};