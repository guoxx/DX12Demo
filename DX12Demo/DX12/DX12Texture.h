#pragma once

#include "DX12GpuResource.h"
#include "DX12DescriptorHandle.h"

class DX12Device;

class DX12Texture : public DX12GpuResource
{
public:
	DX12Texture(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height);
	virtual ~DX12Texture();

	DX12DescriptorHandle GetSRV() const { return m_SRV; }

private:
	void CreateView(DX12Device* device);

	DXGI_FORMAT m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
	uint32_t m_MipLevels;
	DX12DescriptorHandle m_SRV;
};

