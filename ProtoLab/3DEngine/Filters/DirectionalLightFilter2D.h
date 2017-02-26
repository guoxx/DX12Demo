#pragma once

#include "../../DX12/DX12.h"

class RenderContext;
class DirectionalLight;

class DirectionalLightFilter2D
{
public:
	DirectionalLightFilter2D(DX12Device* device);
	~DirectionalLightFilter2D();

	void Apply(DX12GraphicsContext* pGfxContext, const RenderContext* pRenderContext, const DirectionalLight* pLight);
	void Draw(DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

