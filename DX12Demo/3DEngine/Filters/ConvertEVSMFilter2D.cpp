#include "pch.h"
#include "ConvertEVSMFilter2D.h"

#include "../GraphicsEngineDefinition.h"

#include "../../Shaders/CompiledShaders/EVSM.h"


ConvertEVSMFilter2D::ConvertEVSMFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_DESCRIPTOR_RANGE descriptorRanges0[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges0), descriptorRanges0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_EVSM_VS, sizeof(g_EVSM_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_EVSM_PS, sizeof(g_EVSM_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(GFX_FORMAT_R32G32B32A32_FLOAT.RTVFormat);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());
}

ConvertEVSMFilter2D::~ConvertEVSMFilter2D()
{
}

void ConvertEVSMFilter2D::Apply(DX12GraphicContext * pGfxContext)
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

void ConvertEVSMFilter2D::Draw(DX12GraphicContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
