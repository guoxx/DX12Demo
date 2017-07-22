#pragma once

#include "DX12GpuResource.h"
#include "DX12DescriptorHandle.h"

namespace DirectX {
    class ScratchImage;
}

class DX12Device;
class DX12GraphicsContext;

class DX12Texture : public DX12GpuResource
{
public:
	DX12Texture(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height);
	DX12Texture(DX12Device* device, ComPtr<ID3D12Resource> texture, D3D12_RESOURCE_STATES initialState);
	virtual ~DX12Texture();

    static DX12Texture* LoadFromFile(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* filename, bool forceSRGB = false);

    static DX12Texture* LoadFromScratchImage(DX12Device* device, DX12GraphicsContext* pGfxContext, ScratchImage& img, bool forceSRGB = false);

	static DX12Texture* LoadFromBin(DX12Device* device, DX12GraphicsContext* pGfxContext, const uint8_t* pBinData,
		DXGI_FORMAT format, uint32_t width, uint32_t height);

	DX12DescriptorHandle GetSRV() const { return m_SRV; }
	DX12DescriptorHandle GetStagingSRV() const { return m_StagingSRV; }

private:
	void CreateView(DX12Device* device);

	DXGI_FORMAT m_Format;
    uint32_t m_Width;
    uint32_t m_Height;
	uint32_t m_MipLevels;
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_StagingSRV;
};

