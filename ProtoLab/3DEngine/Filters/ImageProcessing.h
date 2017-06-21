#pragma once

#include "../../DX12/DX12.h"
#include <functional>

class ImageProcessing
{
public:
    ImageProcessing(DX12Device* device,
                    D3D12_SHADER_BYTECODE vsBin,
                    D3D12_SHADER_BYTECODE psBin,
                    const std::function<void(DX12GraphicsPsoDesc& desc)>& additionalPsoSetup);
	~ImageProcessing();

	void Apply(DX12GraphicsContext* pGfxContext);
	void Draw(DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

