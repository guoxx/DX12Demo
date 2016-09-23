#pragma once

class DX12GraphicManager;

class DX12Device
{
public:
	DX12Device();
	~DX12Device();

private:
	ComPtr<ID3D12Device> m_d3dDevice;
};

