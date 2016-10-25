#include "pch.h"
#include "LightCullingPass.h"

#include "../Scene.h"
#include "../Camera.h"
#include "../RenderContext.h"

#include "../../Shaders/CompiledShaders/LightCulling_CS.h"


// Keep the same as LightCulling.hlsli
#define LIGHT_CULLING_NUM_THREADS_XY 16

LightCullingPass::LightCullingPass(DX12Device* device)
{
	D3D12_DESCRIPTOR_RANGE descriptorRanges[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
		{ D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2, 0);
	sigCompiler.End();
	sigCompiler[0].InitAsConstantBufferView(0);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges), descriptorRanges, D3D12_SHADER_VISIBILITY_ALL);
	m_RootSig = sigCompiler.Compile(device);

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromBin(DX12ShaderTypeCompute, g_LightCulling_CS, sizeof(g_LightCulling_CS));
	psoCompiler.SetRoogSignature(m_RootSig.get());
	m_PSO = psoCompiler.Compile(device);
}

LightCullingPass::~LightCullingPass()
{
}

void LightCullingPass::Apply(DX12GraphicContext * pGfxContext, const RenderContext* pRenderContext, const Scene* pScene)
{
	pGfxContext->SetGraphicsRootSignature(m_RootSig);
	pGfxContext->SetPipelineState(m_PSO.get());

	struct Constants
	{
		uint32_t m_NumPointLights;
		uint32_t m_NumTileX;
		uint32_t m_NumTileY;
		uint32_t m_Padding;
		float4x4 m_mInvViewProj;
		float4 m_InvScreenSize;
	};
	Constants constants;

	constants.m_NumPointLights = static_cast<uint32_t>(pScene->GetPointLights().size());
	constants.m_NumTileX = (pRenderContext->GetScreenWidth() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;
	constants.m_NumTileY = (pRenderContext->GetScreenHeight() + LIGHT_CULLING_NUM_THREADS_XY - 1) / LIGHT_CULLING_NUM_THREADS_XY;

	DirectX::XMMATRIX mViewProj = pRenderContext->GetViewPorjMatrix();
	DirectX::XMMATRIX mInvViewProj = DirectX::XMMatrixInverse(nullptr, mViewProj);
	DirectX::XMStoreFloat4x4(&constants.m_mInvViewProj, mInvViewProj);

	constants.m_InvScreenSize = DirectX::XMFLOAT4{ 1.0f / pRenderContext->GetScreenWidth(), 1.0f / pRenderContext->GetScreenHeight(), 0, 0 };

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));
}

void LightCullingPass::Exec(DX12GraphicContext* pGfxContext)
{
	pGfxContext->DrawIndexed(6, 0);
}
