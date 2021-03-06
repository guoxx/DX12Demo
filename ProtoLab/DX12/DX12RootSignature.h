#pragma once

#include "DX12Constants.h"

class DX12Device;

class DX12RootSignature
{
	friend class DX12RootSignatureCompiler;
    friend class DX12RootSignatureDeserializer;

public:
	DX12RootSignature(ComPtr<ID3D12RootSignature> rootSig);
	~DX12RootSignature();

	ID3D12RootSignature* GetSignature() const { return m_RootSig.Get(); }

	uint32_t GetDescriptorTableSize(int32_t rootParameterIndex) const
	{
		assert(rootParameterIndex < DX12MaxSlotsPerShader);
		return m_DescriptorTableSize[rootParameterIndex];
	}

private:
	uint32_t m_DescriptorTableSize[DX12MaxSlotsPerShader];
	ComPtr<ID3D12RootSignature> m_RootSig;
};

class DX12RootSignatureCompiler
{
public:
	DX12RootSignatureCompiler();
	~DX12RootSignatureCompiler();

	void Begin(uint32_t numParams);
	void Begin(uint32_t numParams, uint32_t numStaticSamplers);

	void SetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags);
	void UnsetFlag(D3D12_ROOT_SIGNATURE_FLAGS flags);

	void InitStaticSampler(const CD3DX12_STATIC_SAMPLER_DESC& desc);
	void InitStaticSampler(uint32_t shaderRegister, const D3D12_SAMPLER_DESC& staticSamplerDesc, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

	void InitDescriptorTable(uint32_t slot, uint32_t numDescriptorRanges, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	void SetupDescriptorRange(uint32_t slot, uint32_t offsetInTable, const D3D12_DESCRIPTOR_RANGE& descriptorRange);

	CD3DX12_ROOT_PARAMETER& operator[](uint32_t idx)
	{
		assert(idx < m_NumRootParams);
		return m_RootParams.get()[idx];
	}

	void End();

	std::shared_ptr<DX12RootSignature> Compile(DX12Device* device);

private:
	struct DescriptorTableDesc
	{
		uint32_t										m_Num;
		D3D12_SHADER_VISIBILITY							m_Vis;
		std::shared_ptr<D3D12_DESCRIPTOR_RANGE>			m_Ptr;
	};

	enum CompilerState
	{
		StateUnknow,
		StateBegin,
		StateEnd,
		StateCompiled,
	};

	CompilerState m_State;

	D3D12_ROOT_SIGNATURE_FLAGS m_Flags;
	uint32_t m_NumInitializedStaticSamplers;

	uint32_t m_NumRootParams;
	uint32_t m_NumStaticSamplers;
	std::unique_ptr<CD3DX12_ROOT_PARAMETER[]> m_RootParams;
	std::unique_ptr<CD3DX12_STATIC_SAMPLER_DESC[]> m_StaticSamplers;
	std::vector<DescriptorTableDesc> m_CachedDescriptorTables;
};

class DX12RootSignatureDeserializer
{
public:
	DX12RootSignatureDeserializer(D3D12_SHADER_BYTECODE shaderBin);
	DX12RootSignatureDeserializer(ComPtr<ID3DBlob> blob);
	~DX12RootSignatureDeserializer() = default;

	std::shared_ptr<DX12RootSignature> Deserialize(DX12Device* device);    

private:
    ComPtr<ID3DBlob> m_RootSigBlob;
};