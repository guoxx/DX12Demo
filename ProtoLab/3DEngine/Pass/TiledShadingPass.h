#pragma once

#include "../../DX12/DX12.h"

class Scene;
class RenderContext;

class TiledShadingPass
{
public:
	TiledShadingPass(DX12Device* device);
	~TiledShadingPass();

	void Apply(DX12GraphicContext* pGfxContext, const RenderContext* pRenderContext, const Scene* pScene);
	void Exec(DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;

	uint32_t m_NumTileX;
	uint32_t m_NumTileY;
};

