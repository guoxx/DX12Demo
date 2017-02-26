#pragma once

#include "../../DX12/DX12.h"

class RenderContext;
class PointLight;

class PointLightFilter2D
{
public:
	PointLightFilter2D(DX12Device* device);
	~PointLightFilter2D();

	void Apply(DX12GraphicsContext* pGfxContext, const RenderContext* pRenderContext, const PointLight* pPointLight);
	void Draw(DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

