#pragma once

#include "../../DX12/DX12.h"

class ComputeProcessing
{
public:
	ComputeProcessing(DX12Device* device, D3D12_SHADER_BYTECODE shaderBin);
	~ComputeProcessing();

	void Apply(DX12GraphicsContext* pGfxContext);

	void Dispatch(DX12GraphicsContext* pGfxContext, uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ = 1);
	void Dispatch2D(DX12GraphicsContext* pGfxContext, uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
};

