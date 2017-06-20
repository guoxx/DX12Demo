#pragma once

#include "RenderableSurfaceManager.h"

struct GBufferSurfaceSet
{
 	std::array<RenderableSurfaceHandle<DX12ColorSurface>, 4> m_GBuffer;
	RenderableSurfaceHandle<DX12DepthSurface> m_DepthSurface;   

	DX12ColorSurface* GetVelocityBuffer() const;
    void TransmitToRead(DX12GraphicsContext* pCtx) const;
    void TransmitToWrite(DX12GraphicsContext* pCtx) const;
    uint32_t SetAsSRV(DX12GraphicsContext* pCtx, uint32_t rootParameterIndex, uint32_t offsetInTable) const;
    void SetAsRTV(DX12GraphicsContext* pCtx) const;
};

struct PointLightShadowMapSet
{
	std::array<RenderableSurfaceHandle<DX12DepthSurface>, 6> m_ShadowMaps;

    void TransmitToRead(DX12GraphicsContext* pCtx) const;
    void TransmitToWrite(DX12GraphicsContext* pCtx) const;
    uint32_t SetAsSRV(DX12GraphicsContext* pCtx, uint32_t rootParameterIndex, uint32_t offsetInTable) const;
};

struct PostProcessSurfaceSet
{
 	RenderableSurfaceHandle<DX12ColorSurface> m_HDRSurface;
};