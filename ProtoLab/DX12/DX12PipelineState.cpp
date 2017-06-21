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

DX12GraphicsPsoDesc::DX12GraphicsPsoDesc()
    : m_PsoDesc{D3D12_DEFAULT}
{
}

DX12GraphicsPsoDesc::~DX12GraphicsPsoDesc()
{
}

bool DX12GraphicsPsoDesc::SetRoogSignature(DX12RootSignature * rootSig)
{
	m_RootSig = rootSig->GetSignature();
	m_PsoDesc.pRootSignature = m_RootSig.Get();
	return m_PsoDesc.pRootSignature != nullptr;
}

bool DX12GraphicsPsoDesc::SetShaderFromBin(DX12ShaderType shaderType, D3D12_SHADER_BYTECODE shaderBin)
{
	switch (shaderType)
	{
	case DX12ShaderTypeVertex:
		m_PsoDesc.VS = shaderBin;
		break;
	case DX12ShaderTypeGeometry:
		m_PsoDesc.GS = shaderBin;
		break;
	case DX12ShaderTypePixel:
		m_PsoDesc.PS = shaderBin;
		break;
	case DX12ShaderTypeCompute:
		assert(false);
		break;
	default:
		break;
	}
	return true;
}

bool DX12GraphicsPsoDesc::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
{
	m_PsoDesc.RasterizerState = rasterizerDesc;
	return true;
}

bool DX12GraphicsPsoDesc::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
{
	m_PsoDesc.BlendState = blendDesc;
	return true;
}

bool DX12GraphicsPsoDesc::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC & depthStencilDesc)
{
	m_PsoDesc.DepthStencilState = depthStencilDesc;
	return true;
}

bool DX12GraphicsPsoDesc::SetRenderTargetFormats(uint32_t numRenderTargets, DXGI_FORMAT* fmts)
{
	assert(numRenderTargets < DX12MaxRenderTargetsCount);
	m_PsoDesc.NumRenderTargets = numRenderTargets;
	for (uint32_t i = 0; i < numRenderTargets; ++i)
	{
		m_PsoDesc.RTVFormats[i] = fmts[i];
	}
	return true;
}

bool DX12GraphicsPsoDesc::SetDespthStencilFormat(DXGI_FORMAT fmt)
{
	m_PsoDesc.DSVFormat = fmt;
	return true;
}

bool DX12GraphicsPsoDesc::SetInputLayout(uint32_t numElements, const D3D12_INPUT_ELEMENT_DESC * pInputElementDescs)
{
	m_PsoDesc.InputLayout = { pInputElementDescs, numElements };
	return true;
}

bool DX12GraphicsPsoDesc::SetRenderTargetFormatInternal(DXGI_FORMAT fmt)
{
    m_PsoDesc.RTVFormats[m_PsoDesc.NumRenderTargets] = fmt;
    m_PsoDesc.NumRenderTargets += 1;
    return true;
}

DX12ComputePsoDesc::DX12ComputePsoDesc()
{
	std::memset(&m_PsoDesc, 0, sizeof(m_PsoDesc));
}

DX12ComputePsoDesc::~DX12ComputePsoDesc()
{
}

bool DX12ComputePsoDesc::SetRoogSignature(DX12RootSignature * rootSig)
{
	m_RootSig = rootSig->GetSignature();
	m_PsoDesc.pRootSignature = m_RootSig.Get();
	return m_PsoDesc.pRootSignature != nullptr;
}

bool DX12ComputePsoDesc::SetShaderFromBin(D3D12_SHADER_BYTECODE shaderBin)
{
	m_PsoDesc.CS = shaderBin;
	return true;
}

std::shared_ptr<DX12PipelineState> DX12PsoCompiler::Compile(DX12Device* device, DX12GraphicsPsoDesc* pDesc)
{
	return std::make_shared<DX12PipelineState>(device->CreateGraphicsPipelineState(&pDesc->m_PsoDesc));
}

std::shared_ptr<DX12PipelineState> DX12PsoCompiler::Compile(DX12Device* device, DX12ComputePsoDesc* pDesc)
{
	return std::make_shared<DX12PipelineState>(device->CreateComputePipelineState(&pDesc->m_PsoDesc));
}
