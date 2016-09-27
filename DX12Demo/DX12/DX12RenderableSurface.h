#pragma once

#include "DX12GpuResource.h"

class DX12Device;

class DX12RenderableSurface : public DX12GpuResource
{
public:
	DX12RenderableSurface(DX12Device* device, uint32_t width, uint32_t height, DXGI_FORMAT fmt);
	virtual ~DX12RenderableSurface();
};

