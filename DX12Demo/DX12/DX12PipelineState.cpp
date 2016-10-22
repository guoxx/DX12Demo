#include "pch.h"
#include "DX12PipelineState.h"
#include "DX12RootSignature.h"

#include "DX12Device.h"

DX12PipelineState::DX12PipelineState(ComPtr<ID3D12PipelineState> pso)
	: m_PSO{ pso }
{
}


DX12PipelineState::~DX12PipelineState()
{
}

DX12GraphicPsoCompiler::DX12GraphicPsoCompiler()
{
	std::memset(&m_PsoDesc, 0, sizeof(m_PsoDesc));

	m_PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_PsoDesc.SampleMask = UINT_MAX;
	m_PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	m_PsoDesc.InputLayout = D3D12_INPUT_LAYOUT_DESC{ nullptr, 0 };
	m_PsoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	m_PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_PsoDesc.NumRenderTargets = 1;
	m_PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_PsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	m_PsoDesc.SampleDesc = { 1, 0 };
	m_PsoDesc.NodeMask = 0;
#ifdef _XBOX_ONE
	m_PsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
#endif
}

DX12GraphicPsoCompiler::~DX12GraphicPsoCompiler()
{
}

bool DX12GraphicPsoCompiler::SetRoogSignature(DX12RootSignature * rootSig)
{
	m_RootSig = rootSig->GetSignature();
	m_PsoDesc.pRootSignature = m_RootSig.Get();
	return m_PsoDesc.pRootSignature != nullptr;
}

bool DX12GraphicPsoCompiler::SetShaderFromFile(DX12ShaderType shaderType, const wchar_t* file, const char* entry)
{
	assert(shaderType != DX12ShaderTypeCompute);

	static char* profiles[DX12ShaderTypeMax] = {
		"vs_5_1",
		"gs_5_1",
		"ps_5_1",
		"cs_5_1",
	};
	ComPtr<ID3DBlob> bin = CompileShader(file, entry, profiles[shaderType]);
	if (bin.Get() == nullptr)
	{
		assert(false);
		return false;
	}

	m_ShaderBins[shaderType] = bin;
	switch (shaderType)
	{
	case DX12ShaderTypeVertex:
		m_PsoDesc.VS = { bin->GetBufferPointer(), bin->GetBufferSize() };
		break;
	case DX12ShaderTypeGeometry:
		m_PsoDesc.GS = { bin->GetBufferPointer(), bin->GetBufferSize() };
		break;
	case DX12ShaderTypePixel:
		m_PsoDesc.PS = { bin->GetBufferPointer(), bin->GetBufferSize() };
		break;
	case DX12ShaderTypeCompute:
		assert(false);
		break;
	default:
		break;
	}
	return true;
}

bool DX12GraphicPsoCompiler::SetRasterizerState(D3D12_RASTERIZER_DESC & rasterizerDesc)
{
	m_PsoDesc.RasterizerState = rasterizerDesc;
	return true;
}

bool DX12GraphicPsoCompiler::SetBlendState(D3D12_BLEND_DESC & blendDesc)
{
	m_PsoDesc.BlendState = blendDesc;
	return true;
}

bool DX12GraphicPsoCompiler::SetRenderTargetFormat(DXGI_FORMAT fmt0)
{
	m_PsoDesc.NumRenderTargets = 1;
	m_PsoDesc.RTVFormats[0] = fmt0;
	return true;
}

bool DX12GraphicPsoCompiler::SetRenderTargetFormat(DXGI_FORMAT fmt0, DXGI_FORMAT fmt1)
{
	m_PsoDesc.NumRenderTargets = 2;
	m_PsoDesc.RTVFormats[0] = fmt0;
	m_PsoDesc.RTVFormats[1] = fmt1;
	return true;
}

bool DX12GraphicPsoCompiler::SetRenderTargetFormat(DXGI_FORMAT fmt0, DXGI_FORMAT fmt1, DXGI_FORMAT fmt2)
{
	m_PsoDesc.NumRenderTargets = 3;
	m_PsoDesc.RTVFormats[0] = fmt0;
	m_PsoDesc.RTVFormats[1] = fmt1;
	m_PsoDesc.RTVFormats[2] = fmt2;
	return true;
}

bool DX12GraphicPsoCompiler::SetRenderTargetFormats(uint32_t numRenderTargets, DXGI_FORMAT* fmts)
{
	assert(numRenderTargets < DX12MaxRenderTargetsCount);
	m_PsoDesc.NumRenderTargets = numRenderTargets;
	for (uint32_t i = 0; i < numRenderTargets; ++i)
	{
		m_PsoDesc.RTVFormats[i] = fmts[i];
	}
	return true;
}

bool DX12GraphicPsoCompiler::SetDespthStencilFormat(DXGI_FORMAT fmt)
{
	m_PsoDesc.DSVFormat = fmt;
	return true;
}

std::shared_ptr<DX12PipelineState> DX12GraphicPsoCompiler::Compile(DX12Device* device)
{
	return std::make_shared<DX12PipelineState>(device->CreateGraphicsPipelineState(&m_PsoDesc));
}

ComPtr<ID3DBlob> DX12GraphicPsoCompiler::CompileShader(const wchar_t* file, const char* entry, const char* profile) const
{
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	ComPtr<ID3DBlob> errBlob = nullptr;

	uint32_t compileFlags = 0;
	// TODO: fail to use those flags on X1
#ifndef _XBOX_ONE
#ifdef _DEBUG
	compileFlags |= (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION);
#endif
#endif
	D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile, compileFlags, 0, &shaderBlob, &errBlob);
	if (D3DCompileFromFile(file, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry, profile, compileFlags, 0, &shaderBlob, &errBlob) != S_OK)
	{
		char* errMsg = static_cast<char*>(errBlob->GetBufferPointer());
		DX::Print("%s\n", errMsg);
	}
	return shaderBlob;
}

