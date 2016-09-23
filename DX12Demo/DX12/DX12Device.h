#pragma once

class DX12Device
{
public:
	static void Initialize();
	static void Finalize();

	static DX12Device* GetInstance() { return s_Device; }

private:
	DX12Device();
	~DX12Device();

	ComPtr<ID3D12Device> m_d3dDevice;

private:
	static DX12Device* s_Device;
};

