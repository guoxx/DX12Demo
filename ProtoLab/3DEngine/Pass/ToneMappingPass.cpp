#include "pch.h"
#include "ToneMappingPass.h"

#include "3DEngine/Filters/ImageProcessing.h"
#include "3DEngine/GraphicsEngineDefinition.h"

#include "../../Shaders/CompiledShaders/ToneMapFilter2D.h"

ToneMappingPass::ToneMappingPass(DX12Device* device)
{
    auto psoSetup = [](DX12GraphicsPsoDesc& desc)
    {
        desc.SetRenderTargetFormat(GFX_FORMAT_LDR.RTVFormat);
    };
    m_Processing = std::make_shared<ImageProcessing>(device, g_ToneMapFilter2D_VS_bytecode, g_ToneMapFilter2D_PS_bytecode, psoSetup);
}

ToneMappingPass::~ToneMappingPass()
{
}

void ToneMappingPass::Apply(DX12GraphicsContext* pGfxContext,
                            const RenderContext* pRenderContext,
                            PostProcessSurfaceSet& postProcessSurfSet)
{
    pGfxContext->PIXBeginEvent(L"ToneMap");

    m_Processing->Apply(pGfxContext);

    pGfxContext->SetRenderTarget(postProcessSurfSet.m_LDRSurface.Get());

	pGfxContext->SetViewport(0, 0, postProcessSurfSet.m_LDRSurface->GetWidth(), postProcessSurfSet.m_LDRSurface->GetHeight());

    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_AASurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_LuminanceSurfaces.back().Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_LDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    HLSL::CameraSettings constants;
    constants.m_ToneMapEnabled = g_ToneMapping;
    constants.m_ExposureMode = ExposureModes_Automatic;
    constants.m_KeyValue = g_ToneMapTargetLuminance;
    pGfxContext->SetGraphicsRootDynamicConstantBufferView(0, &constants, sizeof(constants));

    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 0, postProcessSurfSet.m_AASurface->GetStagingSRV().GetCpuHandle());
    pGfxContext->SetGraphicsDynamicCbvSrvUav(1, 1, postProcessSurfSet.m_LuminanceSurfaces.back()->GetStagingSRV().GetCpuHandle());

    m_Processing->Draw(pGfxContext);

    pGfxContext->PIXEndEvent();
}
