#include "pch.h"
#include "TiledShadingPass.h"

#include "../Scene.h"
#include "../Camera.h"
#include "../RenderContext.h"

#include "../../Shaders/CompiledShaders/TiledShading.h"


TiledShadingPass::TiledShadingPass(DX12Device* device)
{
	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(6);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsShaderResourceView(0);
	sigCompiler[2].InitAsShaderResourceView(1);
	sigCompiler[3].InitAsShaderResourceView(2);
	sigCompiler.InitDescriptorTable(4, 2);
	sigCompiler.SetupDescriptorRange(4, 0, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 3});
	sigCompiler.SetupDescriptorRange(4, 1, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0});
	sigCompiler.InitDescriptorTable(5, 1);
	sigCompiler.SetupDescriptorRange(5, 0, CD3DX12_DESCRIPTOR_RANGE{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 16});
	m_RootSig = sigCompiler.Compile(device);

	DX12ComputePsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(g_TiledShading_CS, sizeof(g_TiledShading_CS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	m_PSO = psoCompiler.Compile(device);
}

TiledShadingPass::~TiledShadingPass()
{
}

void TiledShadingPass::Apply(DX12GraphicContext * pGfxContext, const RenderContext* pRenderContext, const Scene* pScene)
{
	m_NumTileX = (pRenderContext->GetScreenWidth() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	m_NumTileY = (pRenderContext->GetScreenHeight() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;

	pGfxContext->SetComputeRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());

	HLSL::TiledShadingConstants constants;
	constants.m_NumTileX = m_NumTileX;
	constants.m_NumTileY = m_NumTileY;
	constants.m_ScreenWidth = pRenderContext->GetScreenWidth();
	constants.m_ScreenHeight = pRenderContext->GetScreenHeight();
	constants.m_NumDirectionalLights = static_cast<uint32_t>(pScene->GetDirectionalLights().size());
	DirectX::XMStoreFloat4(&constants.m_CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.m_mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.m_mInvProj, DirectX::XMMatrixTranspose(mInvProj));

	if (g_RSMEnabled)
	{
		constants.m_RSM.m_Enabled = 1;
		constants.m_RSM.m_SampleRadius = g_RSMSampleRadius;
		constants.m_RSM.m_RSMFactor = g_RSMFactor;
		constants.m_RSM.m_RadiusEnd = g_RSMRadiusEnd;
	}
	else
	{
		constants.m_RSM.m_Enabled = 0;
	}

	constants.m_EVSM.m_Enabled = g_EVSMEnabled ? 1 : 0;
	constants.m_EVSM.m_PositiveExponent = g_EVSMPositiveExponent;
	constants.m_EVSM.m_NegativeExponent = g_EVSMNegativeExponent;
	constants.m_EVSM.m_LightBleedingReduction = g_LightBleedingReduction;
	constants.m_EVSM.m_VSMBias = g_VSMBias;

	pGfxContext->SetComputeRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void TiledShadingPass::Exec(DX12GraphicContext* pGfxContext)
{
	pGfxContext->Dispatch(m_NumTileX, m_NumTileY, 1);
}
