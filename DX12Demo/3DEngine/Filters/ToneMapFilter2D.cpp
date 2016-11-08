#include "pch.h"
#include "ToneMapFilter2D.h"

#include "../../Shaders/CompiledShaders/ToneMapFilter2D.h"


ToneMapFilter2D::ToneMapFilter2D(DX12Device* device)
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
	sigCompiler.Begin(2, 1);
	sigCompiler[0].InitAsConstants(1, 0);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges0), descriptorRanges0, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_STATIC_SAMPLER_DESC staticSampDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
	staticSampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sigCompiler.InitStaticSampler(staticSampDesc);
	sigCompiler.End();
	m_RootSig = sigCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_ToneMapFilter2D_VS, sizeof(g_ToneMapFilter2D_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_ToneMapFilter2D_PS, sizeof(g_ToneMapFilter2D_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(GFX_FORMAT_SWAPCHAIN.RTVFormat);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(DX12GraphicManager::GetInstance()->GetDevice());
}

ToneMapFilter2D::~ToneMapFilter2D()
{
}

void ToneMapFilter2D::Apply(DX12GraphicContext * pGfxContext)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ToneMapFilter2D::Draw(DX12GraphicContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
