#pragma once

#include "../../DX12/DX12.h"
#include "3DEngine/SurfaceSet.h"

class RenderContext;
class DirectionalLight;
class ImageProcessing;

class DirectionalLightShadingPass
{
public:
	DirectionalLightShadingPass(DX12Device* device);
	~DirectionalLightShadingPass();

    void Apply(DX12GraphicsContext* pGfxContext,
               const RenderContext* pRenderContext,
               const DirectionalLight* pLight,
               GBufferSurfaceSet& gbuffer, DirectionalLightShadowMapSet& shadowmaps, PostProcessSurfaceSet& postProcessSurfSet);

private:
    std::shared_ptr<ImageProcessing> m_Processing;
};

