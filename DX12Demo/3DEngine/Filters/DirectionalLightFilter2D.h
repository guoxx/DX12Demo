#pragma once

#include "../../DX12/DX12.h"

class RenderContext;
class PointLight;
class DirectionalLight;

class DirectionalLightFilter2D
{
public:
	DirectionalLightFilter2D(DX12Device* device);
	~DirectionalLightFilter2D();

	void Apply(DX12GraphicContext* pGfxContext, const RenderContext* pRenderContext, const DirectionalLight* pLight, const PointLight* pPointLight);
	void Draw(DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

