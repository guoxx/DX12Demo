#include "pch.h"
#include "DX12RootSignature.h"

#include "DX12Device.h"


DX12RootSignature::DX12RootSignature(ComPtr<ID3D12RootSignature> rootSig)
	: m_RootSig{ rootSig }
{
}

DX12RootSignature::~DX12RootSignature()
{
}

DX12RootSignatureCompiler::DX12RootSignatureCompiler()
	: m_State{ StateUnknow }
	, m_Flags{ D3D12_ROOT_SIGNATURE_FLAG_NONE }
	, m_NumInitializedStaticSamplers{ 0 }
{
}

DX12RootSignatureCompiler::~DX12RootSignatureCompiler()
{
}

void DX12RootSignatureCompiler::Begin(uint32_t numParams, uint32_t numStaticSamplers)
{
	assert(m_State == StateUnknow);
	m_State = StateBegin;

	m_NumRootParams = numParams;
	m_NumStaticSamplers = numStaticSamplers;

	if (numParams > 0)
	{
		m_RootParams.reset(new CD3DX12_ROOT_PARAMETER[numParams]);
	}
	else
	{
		m_RootParams = nullptr;
	}

	if (numStaticSamplers)
	{
		m_StaticSamplers.reset(new CD3DX12_STATIC_SAMPLER_DESC[numParams]);
	}
	else
	{
		m_StaticSamplers = nullptr;
	}
}

void DX12RootSignatureCompiler::SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	m_Flags |= flags;
}

void DX12RootSignatureCompiler::UnsetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	m_Flags &= ~flags;
}

void DX12RootSignatureCompiler::InitStaticSampler(const CD3DX12_STATIC_SAMPLER_DESC& desc)
{
	assert(m_NumInitializedStaticSamplers < m_NumStaticSamplers);

	m_StaticSamplers.get()[m_NumInitializedStaticSamplers] = desc;
	m_NumInitializedStaticSamplers += 1;
}

void DX12RootSignatureCompiler::InitStaticSampler(uint32_t shaderRegister, const D3D12_SAMPLER_DESC& staticSamplerDesc, D3D12_SHADER_VISIBILITY visibility)
{
	assert(m_NumInitializedStaticSamplers < m_NumStaticSamplers);
	D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = m_StaticSamplers.get()[m_NumInitializedStaticSamplers];
	m_NumInitializedStaticSamplers += 1;

	StaticSamplerDesc.Filter = staticSamplerDesc.Filter;
	StaticSamplerDesc.AddressU = staticSamplerDesc.AddressU;
	StaticSamplerDesc.AddressV = staticSamplerDesc.AddressV;
	StaticSamplerDesc.AddressW = staticSamplerDesc.AddressW;
	StaticSamplerDesc.MipLODBias = staticSamplerDesc.MipLODBias;
	StaticSamplerDesc.MaxAnisotropy = staticSamplerDesc.MaxAnisotropy;
	StaticSamplerDesc.ComparisonFunc = staticSamplerDesc.ComparisonFunc;
	StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	StaticSamplerDesc.MinLOD = staticSamplerDesc.MinLOD;
	StaticSamplerDesc.MaxLOD = staticSamplerDesc.MaxLOD;
	StaticSamplerDesc.ShaderRegister = shaderRegister;
	StaticSamplerDesc.RegisterSpace = 0;
	StaticSamplerDesc.ShaderVisibility = visibility;

	if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
	{
		if (staticSamplerDesc.BorderColor[3] == 1.0f)
		{
			if (staticSamplerDesc.BorderColor[0] == 1.0f)
			{
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			}
			else
			{
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			}
		}
		else
		{
			StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		}
	}
}

void DX12RootSignatureCompiler::End()
{
	assert(m_State == StateBegin);
	m_State = StateEnd;
}

std::shared_ptr<DX12RootSignature> DX12RootSignatureCompiler::Compile(DX12Device * device)
{
	assert(m_State == StateEnd);
	m_State = StateCompiled;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;	
	rootSignatureDesc.NumParameters = m_NumRootParams;
	rootSignatureDesc.pParameters = m_RootParams.get();
	rootSignatureDesc.NumStaticSamplers = m_NumStaticSamplers;
	rootSignatureDesc.pStaticSamplers = m_StaticSamplers.get();
	rootSignatureDesc.Flags = m_Flags;

	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errBlob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errBlob);

	ComPtr<ID3D12RootSignature> rootSig = device->CreateRootSignature(rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize());
	return std::make_shared<DX12RootSignature>(rootSig);
}

