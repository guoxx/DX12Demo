#pragma once

#include "../../DX12/DX12.h"

class Filter2D
{
public:
	Filter2D(DX12Device* device);
	~Filter2D();

	void Apply(DX12GraphicContext* pGfxContext);
	void Draw(DX12GraphicContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

