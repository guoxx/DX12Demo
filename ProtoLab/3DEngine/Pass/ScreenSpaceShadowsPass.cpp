#include "pch.h"
#include "ScreenSpaceShadowsPass.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/DirectionalLight.h"

#include "3DEngine/Filters/ImageProcessing.h"

#include "../../Shaders/CompiledShaders/ScreenSpaceShadows.h"


ScreenSpaceShadowsPass::ScreenSpaceShadowsPass(DX12Device* device)
{
    m_Processing = std::make_shared<ImageProcessing>(device,
                                                     g_ScreenSpaceShadows_VS_bytecode,
                                                     g_ScreenSpaceShadows_PS_bytecode,
                                                     [](DX12GraphicsPsoDesc& desc)
                                                 {
                                                     desc.SetRenderTargetFormat(GFX_FORMAT_R8G8B8A8_UNORM.RTVFormat);
                                                 });
}

ScreenSpaceShadowsPass::~ScreenSpaceShadowsPass()
{
}

void ScreenSpaceShadowsPass::Apply(DX12GraphicsContext* pGfxContext,
                                  const RenderContext* pRenderContext,
                                  const DirectionalLight* pLight,
                                  GBufferSurfaceSet& gbuffer, DirectionalLightShadowMapSet& shadowmaps, PostProcessSurfaceSet& postProcessSurfSet)
{
    m_Processing->Apply(pGfxContext);

    gbuffer.TransmitToRead(pGfxContext);
    shadowmaps.TransmitToRead(pGfxContext);
    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_ScreenSpaceShadowsSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pGfxContext->SetRenderTarget(postProcessSurfSet.m_ScreenSpaceShadowsSurface.Get());

	HLSL::ScreenSpaceShadowsConstants constants;

    DirectX::XMMATRIX mViewProj = DirectX::XMMatrixMultiply(pRenderContext->GetViewMatrix(), pRenderContext->GetProjMatrix());
    DirectX::XMMATRIX mInvViewProj = DirectX::XMMatrixInverse(nullptr, mViewProj);
	DirectX::XMStoreFloat4x4(&constants.mInvViewProj, DirectX::XMMatrixTranspose(mInvViewProj));

    for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
    {
        DirectX::XMMATRIX mLightView;
        DirectX::XMMATRIX mLightProj;
        pLight->GetViewAndProjMatrix(cascadeIdx, &mLightView, &mLightProj);
        DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
        DirectX::XMStoreFloat4x4(&constants.m_mCascadeViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mLightViewProj));
    }

	constants.m_EVSM.m_Enabled = g_EVSMEnabled ? 1 : 0;
	constants.m_EVSM.m_PositiveExponent = g_EVSMPositiveExponent;
	constants.m_EVSM.m_NegativeExponent = g_EVSMNegativeExponent;
	constants.m_EVSM.m_LightBleedingReduction = g_LightBleedingReduction;
	constants.m_EVSM.m_VSMBias = g_VSMBias;

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));

    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, gbuffer.m_DepthSurface->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, shadowmaps.m_ShadowMap->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 2, shadowmaps.m_EVSMSurface->GetStagingSRV().GetCpuHandle());

    m_Processing->Draw(pGfxContext);
}
