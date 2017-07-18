#include "pch.h"
#include "DirectionalLightShadingPass.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/DirectionalLight.h"

#include "3DEngine/Filters/ImageProcessing.h"

#include "../../Shaders/CompiledShaders/DirectionalLightShading.h"


DirectionalLightShadingPass::DirectionalLightShadingPass(DX12Device* device)
{
    auto psoSetup = [](DX12GraphicsPsoDesc& desc)
    {
        desc.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
	    desc.SetBlendState(CD3DX12::BlendAdditive());
    };
    m_Processing = std::make_shared<ImageProcessing>(device, g_DirectionalLightShading_VS_bytecode, g_DirectionalLightShading_PS_bytecode, psoSetup);
}

DirectionalLightShadingPass::~DirectionalLightShadingPass()
{
}

void DirectionalLightShadingPass::Apply(DX12GraphicsContext* pGfxContext,
                                  const RenderContext* pRenderContext,
                                  const DirectionalLight* pLight,
                                  GBufferSurfaceSet& gbuffer, DirectionalLightShadowMapSet& shadowmaps, PostProcessSurfaceSet& postProcessSurfSet)
{
    m_Processing->Apply(pGfxContext);

    gbuffer.TransmitToRead(pGfxContext);
    shadowmaps.TransmitToRead(pGfxContext);
    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pGfxContext->SetRenderTarget(postProcessSurfSet.m_HDRSurface.Get());

	HLSL::DirectionalLightConstants constants;

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
    DirectX::XMMATRIX mViewProj = DirectX::XMMatrixMultiply(pRenderContext->GetViewMatrix(), pRenderContext->GetProjMatrix());
    DirectX::XMMATRIX mInvViewProj = DirectX::XMMatrixInverse(nullptr, mViewProj);
	DirectX::XMStoreFloat4x4(&constants.mInvViewProj, DirectX::XMMatrixTranspose(mInvViewProj));
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	DirectX::XMFLOAT4 direction = pLight->GetDirection();
	DirectX::XMFLOAT4 irradiance = pLight->GetIrradiance();
	constants.m_DirLight.m_Direction = DirectX::XMFLOAT3{ direction.x, direction.y, direction.z };
	constants.m_DirLight.m_Irradiance = DirectX::XMFLOAT3{ irradiance.x, irradiance.y, irradiance.z };

    for (int32_t cascadeIdx = 0; cascadeIdx < DirectionalLight::NUM_CASCADED_SHADOW_MAP; ++cascadeIdx)
    {
        DirectX::XMMATRIX mLightView;
        DirectX::XMMATRIX mLightProj;
        pLight->GetViewAndProjMatrix(cascadeIdx, &mLightView, &mLightProj);
        DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
        DirectX::XMStoreFloat4x4(&constants.m_DirLight.m_mViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mLightViewProj));

        DirectX::XMMATRIX mInvLightViewProj = DirectX::XMMatrixInverse(nullptr, mLightViewProj);
        DirectX::XMStoreFloat4x4(&constants.m_DirLight.m_mInvViewProj[cascadeIdx], DirectX::XMMatrixTranspose(mInvLightViewProj));
    }

	constants.m_DirLight.m_ShadowMapTexId = -1;
	constants.m_DirLight.m_RSMIntensityTexId = -1;
	constants.m_DirLight.m_RSMNormalTexId = -1;

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

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));

    uint32_t offsetInTbl = gbuffer.SetAsSRV(pGfxContext, 1, 0);
    offsetInTbl = shadowmaps.SetAsSRV(pGfxContext, 1, offsetInTbl);

    m_Processing->Draw(pGfxContext);
}
