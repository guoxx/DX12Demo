#pragma once

#include "DX12GpuResource.h"

class DX12Device;

class DX12RenderableSurface : public DX12GpuResource
{
public:
	DX12RenderableSurface();

	virtual ~DX12RenderableSurface();

    uint32_t GetWidth() const { assert(m_Width > 0); return m_Width; }
    uint32_t GetHeight() const { assert(m_Height > 0); return m_Height; }

protected:

	void Init(DX12Device* device,
		DXGI_FORMAT fmt,
		uint32_t width,
		uint32_t height,
		uint32_t arraySize,
		uint32_t mipLevels,
        uint32_t sampleCount,
        uint32_t sampleQuality,
        D3D12_RESOURCE_FLAGS flags,
		const D3D12_CLEAR_VALUE* pOptimizedClearValue,
		D3D12_RESOURCE_STATES initialState);

protected:
    uint32_t m_Width;
    uint32_t m_Height;
};

