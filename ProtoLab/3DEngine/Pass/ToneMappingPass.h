#pragma once

#include "../../DX12/DX12.h"
#include "3DEngine/SurfaceSet.h"

class RenderContext;
class PointLight;
class ImageProcessing;

class ToneMappingPass
{
public:
	ToneMappingPass(DX12Device* device);
	~ToneMappingPass();

    void Apply(DX12GraphicsContext* pGfxContext,
               const RenderContext* pRenderContext,
               PostProcessSurfaceSet& postProcessSurfSet);

private:
    std::shared_ptr<ImageProcessing> m_Processing;
};

