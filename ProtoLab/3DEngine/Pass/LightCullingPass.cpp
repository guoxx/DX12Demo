#include "pch.h"
#include "LightCullingPass.h"

#include "../Scene.h"
#include "../Camera.h"
#include "../RenderContext.h"

#include "../../Shaders/CompiledShaders/LightCulling.h"


LightCullingPass::LightCullingPass(DX12Device* device)
{
    DX12RootSignatureDeserializer sigDeserialier{g_LightCulling_CS, sizeof(g_LightCulling_CS)};
	m_RootSig = sigDeserialier.Deserialize(device);

	DX12ComputePsoDesc psoDesc;
	psoDesc.SetShaderFromBin(g_LightCulling_CS, sizeof(g_LightCulling_CS));
	psoDesc.SetRoogSignature(m_RootSig.get());
	m_PSO = DX12PsoCompiler::Compile(device, &psoDesc);
}

LightCullingPass::~LightCullingPass()
{
}

void LightCullingPass::Apply(DX12GraphicsContext * pGfxContext, const RenderContext* pRenderContext, const Scene* pScene)
{
	m_NumTileX = (pRenderContext->GetScreenWidth() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	m_NumTileY = (pRenderContext->GetScreenHeight() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;

	pGfxContext->SetComputeRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());

	HLSL::LightCullingConstants constants;

	constants.m_NumPointLights = static_cast<uint32_t>(pScene->GetPointLights().size());
	constants.m_NumTileX = m_NumTileX;
	constants.m_NumTileY = m_NumTileY;

	DirectX::XMMATRIX mView = pRenderContext->GetViewMatrix();
	DirectX::XMMATRIX mProj = pRenderContext->GetProjMatrix();
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, mProj);
	DirectX::XMStoreFloat4x4(&constants.m_mView, DirectX::XMMatrixTranspose(mView));
	DirectX::XMStoreFloat4x4(&constants.m_mInvProj, DirectX::XMMatrixTranspose(mInvProj));

	constants.m_InvScreenSize = DirectX::XMFLOAT4{ 1.0f / pRenderContext->GetScreenWidth(), 1.0f / pRenderContext->GetScreenHeight(), 0, 0 };

	pGfxContext->SetComputeRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void LightCullingPass::Exec(DX12GraphicsContext* pGfxContext)
{
	pGfxContext->Dispatch(m_NumTileX, m_NumTileY, 1);
}
