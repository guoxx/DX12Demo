#pragma once

class DX12Device;

class DX12PipelineState
{
public:
	DX12PipelineState();
	~DX12PipelineState();

private:
	ComPtr<ID3D12PipelineState> m_PSO;
};

