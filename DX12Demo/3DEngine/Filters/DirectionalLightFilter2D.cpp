#include "pch.h"
#include "DirectionalLightFilter2D.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/DirectionalLight.h"

#include "../../Shaders/CompiledShaders/DirectionalLight_VS.h"
#include "../../Shaders/CompiledShaders/DirectionalLight_PS.h"


DirectionalLightFilter2D::DirectionalLightFilter2D(DX12Device* device)
{
	uint32_t indices[] = {0, 2, 1, 1, 2, 3};

	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(device, sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadBuffer(m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_DESCRIPTOR_RANGE descriptorRanges1[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2, 1);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges1), descriptorRanges1, D3D12_SHADER_VISIBILITY_PIXEL);
	CD3DX12_STATIC_SAMPLER_DESC staticSampDesc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
	staticSampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sigCompiler.InitStaticSampler(staticSampDesc);
	m_RootSig = sigCompiler.Compile(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeVertex, g_DirectionalLight_VS, sizeof(g_DirectionalLight_VS));
	psoCompiler.SetShaderFromBin(DX12ShaderTypePixel, g_DirectionalLight_PS, sizeof(g_DirectionalLight_PS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_UNKNOWN);
	psoCompiler.SetBlendState(CD3DX12::BlendAdditive());
	psoCompiler.SetDepthStencilState(CD3DX12::DepthStateDisabled());
	m_PSO = psoCompiler.Compile(device);
}

DirectionalLightFilter2D::~DirectionalLightFilter2D()
{
}

void DirectionalLightFilter2D::Apply(DX12GraphicContext * pGfxContext, const RenderContext* pRenderContext, const DirectionalLight* pLight)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());
	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	struct Constants
	{
		float4x4 mInvView;
		float4x4 mInvProj;
		float4 LightDirection;
		float4 LightIrradiance;
		float4 CameraPosition;
		float4x4 mLightViewProj;
	};
	Constants constants;

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
	constants.LightDirection = pLight->GetDirection();
	constants.LightIrradiance = pLight->GetIrradiance();
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());
	DirectX::XMMATRIX mLightView;
	DirectX::XMMATRIX mLightProj;
	pLight->GetViewAndProjMatrix(pRenderContext->GetCamera(), &mLightView, &mLightProj);
	DirectX::XMStoreFloat4x4(&constants.mLightViewProj, DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(mLightView, mLightProj)));

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void DirectionalLightFilter2D::Draw(DX12GraphicContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
