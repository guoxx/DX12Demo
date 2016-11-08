#include "pch.h"
#include "Filter2D.h"

#include "../../Shaders/CompiledShaders/IdentityFilter2D.h"


Filter2D::Filter2D(DX12Device* device)
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
	sigCompiler.Begin(1);
	sigCompiler.End();
	sigCompiler[0].InitAsDescriptorTable(_countof(descriptorRanges0), descriptorRanges0, D3D12_SHADER_VISIBILITY_PIXEL);
	m_RootSig = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_IdentityFilter2D_VS, sizeof(g_IdentityFilter2D_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_IdentityFilter2D_PS, sizeof(g_IdentityFilter2D_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());
}

Filter2D::~Filter2D()
{
}

void Filter2D::Apply(DX12GraphicContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Filter2D::Draw(DX12GraphicContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
