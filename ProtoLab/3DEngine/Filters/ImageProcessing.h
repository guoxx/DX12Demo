#pragma once

#include "../../DX12/DX12.h"

class ImageProcessing
{
public:
    ImageProcessing(DX12Device* device,
                    const void* pVSBin, uint32_t vsDataSize,
                    const void* pPSBin, uint32_t psDataSize,
                    DXGI_FORMAT renderTargetFmt);
	~ImageProcessing();

	void Apply(DX12GraphicsContext* pGfxContext);
	void Draw(DX12GraphicsContext* pGfxContext);

private:
	std::shared_ptr<DX12RootSignature> m_RootSig;
	std::shared_ptr<DX12PipelineState> m_PSO;
	std::shared_ptr<DX12IndexBuffer> m_IndexBuffer;
};

