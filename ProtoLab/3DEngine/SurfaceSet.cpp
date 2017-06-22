#include "pch.h"
#include "SurfaceSet.h"


DX12ColorSurface* GBufferSurfaceSet::GetVelocityBuffer() const
{
    return m_GBuffer[3].Get();
}

void GBufferSurfaceSet::TransmitToRead(DX12GraphicsContext* pCtx) const
{
    for (auto surf : m_GBuffer)
    {
        pCtx->ResourceTransitionBarrier(surf.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    }
    pCtx->ResourceTransitionBarrier(m_DepthSurface.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void GBufferSurfaceSet::TransmitToWrite(DX12GraphicsContext* pCtx) const
{
    for (auto surf : m_GBuffer)
    {
        pCtx->ResourceTransitionBarrier(surf.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    }
    pCtx->ResourceTransitionBarrier(m_DepthSurface.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
}

uint32_t GBufferSurfaceSet::SetAsSRV(DX12GraphicsContext* pCtx, uint32_t rootParameterIndex, uint32_t offsetInTable) const
{
    auto idx = offsetInTable;

    for (auto surf : m_GBuffer)
    {
        pCtx->SetGraphicsDynamicCbvSrvUav(rootParameterIndex, idx, surf->GetStagingSRV().GetCpuHandle());
        idx += 1;
    }

    pCtx->SetGraphicsDynamicCbvSrvUav(rootParameterIndex, idx, m_DepthSurface->GetStagingSRV().GetCpuHandle());
    idx  += 1;
    
    return idx;
}

void GBufferSurfaceSet::SetAsRTV(DX12GraphicsContext* pCtx) const
{
	DX12ColorSurface* pColorSurfaces[] = {
		m_GBuffer[0].Get(),
		m_GBuffer[1].Get(),
		m_GBuffer[2].Get(),
		m_GBuffer[3].Get()
	};
    pCtx->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_DepthSurface.Get());
}

void PointLightShadowMapSet::TransmitToRead(DX12GraphicsContext* pCtx) const
{
    for (auto surf : m_ShadowMaps)
    {
        pCtx->ResourceTransitionBarrier(surf.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    }
}

void PointLightShadowMapSet::TransmitToWrite(DX12GraphicsContext* pCtx) const
{
    for (auto surf : m_ShadowMaps)
    {
        pCtx->ResourceTransitionBarrier(surf.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }
}

uint32_t PointLightShadowMapSet::SetAsSRV(DX12GraphicsContext* pCtx, uint32_t rootParameterIndex, uint32_t offsetInTable) const
{
    auto idx = offsetInTable;
    for (auto surf : m_ShadowMaps)
    {
        pCtx->SetGraphicsDynamicCbvSrvUav(rootParameterIndex, idx, surf->GetStagingSRV().GetCpuHandle());
        idx += 1;
    }
    return idx;
}
