#include "pch.h"
#include "ComputeProcessing.h"


ComputeProcessing::ComputeProcessing(DX12Device* device, const void* pCSBin, uint32_t dataSizeInBytes)
{
    DX12RootSignatureDeserializer sigDeserialier{pCSBin, dataSizeInBytes};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12ComputePsoDesc psoDesc;
	psoDesc.SetShaderFromBin(pCSBin, dataSizeInBytes);
	psoDesc.SetRoogSignature(m_RootSig.get());
	m_PSO = DX12PsoCompiler::Compile(device, &psoDesc);
}

ComputeProcessing::~ComputeProcessing()
{
}

void ComputeProcessing::Apply(DX12GraphicsContext * pGfxContext)
{
	pGfxContext->SetComputeRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
}

void ComputeProcessing::Dispatch(DX12GraphicsContext* pGfxContext, uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ)
{
    pGfxContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void ComputeProcessing::Dispatch2D(DX12GraphicsContext* pGfxContext, uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
{
    pGfxContext->Dispatch2D(threadCountX, threadCountY, groupSizeX, groupSizeY);
}

