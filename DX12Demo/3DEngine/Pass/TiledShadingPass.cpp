#include "pch.h"
#include "TiledShadingPass.h"

#include "../Scene.h"
#include "../Camera.h"
#include "../RenderContext.h"

#include "../../Shaders/CompiledShaders/TiledShading_CS.h"


TiledShadingPass::TiledShadingPass(DX12Device* device)
{
	D3D12_DESCRIPTOR_RANGE descriptorRanges[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 3, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
		{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRanges1[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 16, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(6, 1);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsShaderResourceView(0);
	sigCompiler[2].InitAsShaderResourceView(1);
	sigCompiler[3].InitAsShaderResourceView(2);
	sigCompiler[4].InitAsDescriptorTable(_countof(descriptorRanges), descriptorRanges);
	sigCompiler[5].InitAsDescriptorTable(_countof(descriptorRanges1), descriptorRanges1);
	sigCompiler.InitStaticSampler(CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT));
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
