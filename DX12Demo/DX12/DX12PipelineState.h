#pragma once

#include "DX12Constants.h"

class DX12Device;
class DX12RootSignature;

class DX12PipelineState
{
public:
	DX12PipelineState(ComPtr<ID3D12PipelineState> pso);
	~DX12PipelineState();

	ID3D12PipelineState* GetPSO() const { return m_PSO.Get(); }

private:
	ComPtr<ID3D12PipelineState> m_PSO;
};

class DX12GraphicPsoCompiler
{
public:
	DX12GraphicPsoCompiler();
	~DX12GraphicPsoCompiler();

	bool SetRoogSignature(DX12RootSignature* rootSig);

	bool SetShaderFromFile(DX12ShaderType shaderType, const wchar_t* file, const char* entry);

	// optional
	bool SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);

	bool SetBlendState(const D3D12_BLEND_DESC& blendDesc);

	bool SetRenderTargetFormat(DXGI_FORMAT fmt0);
	bool SetRenderTargetFormat(DXGI_FORMAT fmt0, DXGI_FORMAT fmt1);
	bool SetRenderTargetFormat(DXGI_FORMAT fmt0, DXGI_FORMAT fmt1, DXGI_FORMAT fmt2);
	bool SetRenderTargetFormats(uint32_t numRenderTargets, DXGI_FORMAT* fmts);

	bool SetDespthStencilFormat(DXGI_FORMAT fmt);

	std::shared_ptr<DX12PipelineState> Compile(DX12Device* device);

private:
	ComPtr<ID3DBlob> CompileShader(const wchar_t* file, const char* entry, const char* profile) const;

	ComPtr<ID3D12RootSignature> m_RootSig;
	ComPtr<ID3DBlob> m_ShaderBins[DX12ShaderTypeMax];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PsoDesc;
};