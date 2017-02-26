#pragma once

#include "../../DX12/DX12.h"

class ToneMapFilter2D
{
public:
	ToneMapFilter2D(DX12Device* device);
	~ToneMapFilter2D();

	void Apply(DX12GraphicsContext* pGfxContext);
	void Draw(DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

