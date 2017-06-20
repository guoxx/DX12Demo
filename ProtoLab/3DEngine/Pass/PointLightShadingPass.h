#pragma once

#include "../../DX12/DX12.h"
#include "3DEngine/SurfaceSet.h"

class RenderContext;
class PointLight;
class ImageProcessing;

class PointLightShadingPass
{
public:
	PointLightShadingPass(DX12Device* device);
	~PointLightShadingPass();

    void Apply(DX12GraphicsContext* pGfxContext,
               const RenderContext* pRenderContext,
               const PointLight* pPointLight,
               GBufferSurfaceSet& gbuffer, PointLightShadowMapSet& shadowmaps, PostProcessSurfaceSet& postProcessSurfSet);

private:
    std::shared_ptr<ImageProcessing> m_Processing;
};

