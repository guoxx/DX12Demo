#include "pch.h"
#include "DX12RenderableSurface.h"

#include "DX12Device.h"


DX12RenderableSurface::DX12RenderableSurface()
{
}

DX12RenderableSurface::~DX12RenderableSurface()
{
}

void DX12RenderableSurface::Init(DX12Device* device,
	DXGI_FORMAT fmt,
	uint32_t width,
	uint32_t height,
	uint32_t arraySize,
	uint32_t mipLevels,
	uint32_t sampleCount,
	uint32_t sampleQuality,
	D3D12_RESOURCE_FLAGS flags,
	const D3D12_CLEAR_VALUE * pOptimizedClearValue,
	D3D12_RESOURCE_STATES initialState)
{
	ComPtr<ID3D12Resource> res = device->CreateCommittedTexture2DInDefaultHeap(fmt,
		width,
		height,
		arraySize,
		mipLevels,
		sampleCount,
		sampleQuality,
		flags,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		pOptimizedClearValue,
		initialState);

	SetGpuResource(res, initialState);
}
