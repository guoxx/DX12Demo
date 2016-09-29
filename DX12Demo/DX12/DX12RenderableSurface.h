#pragma once

#include "DX12GpuResource.h"

class DX12Device;

class DX12RenderableSurface : public DX12GpuResource
{
public:
	DX12RenderableSurface();

	virtual ~DX12RenderableSurface();

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
};

