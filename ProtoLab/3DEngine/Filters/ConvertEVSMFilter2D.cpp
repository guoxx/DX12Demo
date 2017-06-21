#include "pch.h"
#include "ConvertEVSMFilter2D.h"

#include "../GraphicsEngineDefinition.h"

#include "../../Shaders/CompiledShaders/EVSM.h"


ConvertEVSMFilter2D::ConvertEVSMFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12ScopedGraphicsContext pGfxContext;

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    DX12RootSignatureDeserializer sigDeserialier{g_EVSM_VS, sizeof(g_EVSM_VS)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12GraphicsPsoDesc psoDesc;
	psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, g_EVSM_VS, sizeof(g_EVSM_VS));
	psoDesc.SetShaderFromBin(DX12ShaderTypePixel, g_EVSM_PS, sizeof(g_EVSM_PS));
	psoDesc.SetRoogSignature(m_RootSig.get());
	psoDesc.SetRenderTargetFormat(GFX_FORMAT_R32G32B32A32_FLOAT.RTVFormat);
	psoDesc.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoDesc.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = DX12PsoCompiler::Compile(device, &psoDesc);
}

ConvertEVSMFilter2D::~ConvertEVSMFilter2D()
{
}

void ConvertEVSMFilter2D::Apply(DX12GraphicsContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	HLSL::EVSMConstants constants;
	constants.m_EVSM.m_Enabled = g_EVSMEnabled ? 1 : 0;
	constants.m_EVSM.m_PositiveExponent = g_EVSMPositiveExponent;
	constants.m_EVSM.m_NegativeExponent = g_EVSMNegativeExponent;
	constants.m_EVSM.m_LightBleedingReduction = g_LightBleedingReduction;
	constants.m_EVSM.m_VSMBias = g_VSMBias;
	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void ConvertEVSMFilter2D::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
