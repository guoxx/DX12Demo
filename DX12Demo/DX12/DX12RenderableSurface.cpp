#include "pch.h"
#include "DX12RenderableSurface.h"

#include "DX12Device.h"

DX12RenderableSurface::DX12RenderableSurface(DX12Device* device,
	DXGI_FORMAT fmt,
	uint32_t width,
	uint32_t height,
	uint32_t arraySize,
	uint32_t mipLevels,
	const D3D12_CLEAR_VALUE * pOptimizedClearValue)
{
}

DX12RenderableSurface::~DX12RenderableSurface()
{
}
