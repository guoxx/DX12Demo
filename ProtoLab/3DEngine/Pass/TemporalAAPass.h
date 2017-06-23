#pragma once

#include "../../DX12/DX12.h"
#include "3DEngine/SurfaceSet.h"

class RenderContext;
class PointLight;
class ImageProcessing;

class TemporalAAPass
{
public:
	TemporalAAPass(DX12Device* device);
	~TemporalAAPass();

    void Apply(DX12GraphicsContext* pGfxContext,
               const RenderContext* pRenderContext,
               GBufferSurfaceSet& gbuffer, PostProcessSurfaceSet& postProcessSurfSet);

private:
    std::shared_ptr<ImageProcessing> m_TAA;
    std::shared_ptr<ImageProcessing> m_CopyFP16;
};

