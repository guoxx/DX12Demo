#include "pch.h"
#include "PointLightShadingPass.h"

#include "../Camera.h"
#include "../RenderContext.h"
#include "../Lights/PointLight.h"

#include "3DEngine/Filters/ImageProcessing.h"

#include "../../Shaders/CompiledShaders/PointLight.h"


PointLightShadingPass::PointLightShadingPass(DX12Device* device)
{
    auto psoSetup = [](DX12GraphicsPsoDesc& desc)
    {
        desc.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
    };
    m_Processing = std::make_shared<ImageProcessing>(device, g_PointLight_VS_bytecode, g_PointLight_PS_bytecode, psoSetup);
}

PointLightShadingPass::~PointLightShadingPass()
{
}

void PointLightShadingPass::Apply(DX12GraphicsContext* pGfxContext,
                                  const RenderContext* pRenderContext,
                                  const PointLight* pPointLight,
                                  GBufferSurfaceSet& gbuffer, PointLightShadowMapSet& shadowmaps, PostProcessSurfaceSet& postProcessSurfSet)
{
    m_Processing->Apply(pGfxContext);

    gbuffer.TransmitToRead(pGfxContext);
    shadowmaps.TransmitToRead(pGfxContext);
    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    pGfxContext->SetRenderTarget(postProcessSurfSet.m_HDRSurface.Get());

	struct Constants
	{
		float4x4 mInvView;
		float4x4 mInvProj;
		float4 CameraPosition;

		float4 LightPositionAndRadius;
		float4 LightRadiantPower;
		float4 LightRadius;
		float4x4 mLightViewProj[6];
	};
	Constants constants;

	DirectX::XMMATRIX mInvView = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetViewMatrix());
	DirectX::XMMATRIX mInvProj = DirectX::XMMatrixInverse(nullptr, pRenderContext->GetProjMatrix());
	DirectX::XMStoreFloat4x4(&constants.mInvView, DirectX::XMMatrixTranspose(mInvView));
	DirectX::XMStoreFloat4x4(&constants.mInvProj, DirectX::XMMatrixTranspose(mInvProj));
	DirectX::XMStoreFloat4(&constants.CameraPosition, pRenderContext->GetCamera()->GetTranslation());

	DirectX::XMStoreFloat4(&constants.LightPositionAndRadius, pPointLight->GetTranslation());
    constants.LightPositionAndRadius.w = pPointLight->GetRadius();
	DirectX::XMStoreFloat4(&constants.LightRadiantPower, pPointLight->GetRadiantPower());
	for (int i = 0; i < 6; ++i)
	{
		DirectX::XMMATRIX mLightView;
		DirectX::XMMATRIX mLightProj;
		pPointLight->GetViewAndProjMatrix((PointLight::AXIS)i, &mLightView, &mLightProj);
		DirectX::XMMATRIX mLightViewProj = DirectX::XMMatrixMultiply(mLightView, mLightProj);
		DirectX::XMStoreFloat4x4(&constants.mLightViewProj[i], DirectX::XMMatrixTranspose(mLightViewProj));
	}

	pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));

    uint32_t offsetInTbl = gbuffer.SetAsSRV(pGfxContext, 1, 0);
    offsetInTbl = shadowmaps.SetAsSRV(pGfxContext, 1, offsetInTbl);

    m_Processing->Draw(pGfxContext);
}
