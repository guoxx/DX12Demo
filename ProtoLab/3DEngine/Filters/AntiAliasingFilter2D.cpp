#include "pch.h"
#include "AntiAliasingFilter2D.h"

#include "../../Shaders/CompiledShaders/AntiAliasingFilter2D.h"


AntiAliasingFilter2D::AntiAliasingFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12ScopedGraphicsContext pGfxContext;

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

    DX12RootSignatureDeserializer sigDeserialier{g_AntiAliasingFilter2D_VS, sizeof(g_AntiAliasingFilter2D_VS)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12GraphicsPsoDesc psoDesc;
	psoDesc.SetShaderFromBin(DX12ShaderTypeVertex, g_AntiAliasingFilter2D_VS, sizeof(g_AntiAliasingFilter2D_VS));
	psoDesc.SetShaderFromBin(DX12ShaderTypePixel, g_AntiAliasingFilter2D_PS, sizeof(g_AntiAliasingFilter2D_PS));
	psoDesc.SetRoogSignature(m_RootSig.get());
	psoDesc.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
	psoDesc.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoDesc.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = DX12PsoCompiler::Compile(device, &psoDesc);
}

AntiAliasingFilter2D::~AntiAliasingFilter2D()
{
}

void AntiAliasingFilter2D::Apply(DX12GraphicsContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void AntiAliasingFilter2D::Draw(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
