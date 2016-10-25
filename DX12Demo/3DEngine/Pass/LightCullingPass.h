#pragma once

#include "../../DX12/DX12.h"

class Scene;
class RenderContext;

class LightCullingPass
{
public:
	LightCullingPass(DX12Device* device);
	~LightCullingPass();

	void Apply(DX12GraphicContext* pGfxContext, const RenderContext* pRenderContext, const Scene* pScene);
	void Exec(DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
};

