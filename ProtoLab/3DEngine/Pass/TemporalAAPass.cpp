#include "pch.h"
#include "TemporalAAPass.h"

#include "3DEngine/Filters/ImageProcessing.h"
#include "3DEngine/GraphicsEngineDefinition.h"

#include <Shaders/CompiledShaders/AntiAliasingFilter.h>
#include <Shaders/CompiledShaders/Passthrough.h>

TemporalAAPass::TemporalAAPass(DX12Device* device)
{
    {
        auto psoSetup = [](DX12GraphicsPsoDesc& desc)
        {
            desc.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
        };
        m_TAA = std::make_shared<ImageProcessing>(device, g_AntiAliasingFilter_VS_bytecode, g_AntiAliasingFilter_PS_bytecode, psoSetup);
    }

	{
        auto psoSetup = [](DX12GraphicsPsoDesc& desc)
        {
            desc.SetRenderTargetFormat(GFX_FORMAT_HDR.RTVFormat);
        };
        m_CopyFP16 = std::make_shared<ImageProcessing>(device, g_Passthrough_VS_bytecode, g_Passthrough_PS_bytecode, psoSetup);
	}
}

TemporalAAPass::~TemporalAAPass()
{
}

void TemporalAAPass::Apply(DX12GraphicsContext* pGfxContext,
                            const RenderContext* pRenderContext,
                            GBufferSurfaceSet& gbuffer, PostProcessSurfaceSet& postProcessSurfSet)
{
    pGfxContext->PIXBeginEvent(L"TemporalAA");

    pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pGfxContext->SetViewport(0, 0, postProcessSurfSet.m_AASurface->GetWidth(), postProcessSurfSet.m_AASurface->GetHeight());

    {
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HistoryHDRSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(gbuffer.GetVelocityBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_AASurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_TAA->Apply(pGfxContext);
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 0, postProcessSurfSet.m_HDRSurface->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 1, postProcessSurfSet.m_HistoryHDRSurface->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 2, gbuffer.GetVelocityBuffer()->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetRenderTarget(postProcessSurfSet.m_AASurface.Get());
        m_TAA->Draw(pGfxContext);
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    {
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_AASurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
        pGfxContext->ResourceTransitionBarrier(postProcessSurfSet.m_HistoryHDRSurface.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_CopyFP16->Apply(pGfxContext);
        pGfxContext->SetGraphicsDynamicCbvSrvUav(0, 0, postProcessSurfSet.m_AASurface->GetStagingSRV().GetCpuHandle());
        pGfxContext->SetRenderTarget(postProcessSurfSet.m_HistoryHDRSurface.Get());
        m_CopyFP16->Draw(pGfxContext);
    }

    pGfxContext->PIXEndEvent();
}
