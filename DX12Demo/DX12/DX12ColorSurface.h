#pragma once

#include "DX12Wrapper.h"
#include "DX12RenderableSurface.h"
#include "DX12DescriptorHandle.h"

class DX12ColorSurface : public DX12RenderableSurface
{
public:
	DX12ColorSurface();
	virtual ~DX12ColorSurface();

	void InitAs2dSurface(DX12Device* device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height);
	void InitAs2dSurface(DX12Device* device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height, uint32_t mipLevels);
	void InitAs2dSurface(DX12Device* device, ComPtr<ID3D12Resource> pResource, GFX_FORMAT_SET fmt, D3D12_RESOURCE_STATES usageState);

	void InitAsCubeSurface(DX12Device* device, GFX_FORMAT_SET fmt, uint32_t size);
	void InitAsCubeSurface(DX12Device* device, GFX_FORMAT_SET fmt, uint32_t size, uint32_t mipLevels);

	DX12DescriptorHandle GetSRV() const { return m_SRV; };
	DX12DescriptorHandle GetRTV() const { return m_RTV; };

	DX12DescriptorHandle GetStagingSRV() const { return m_StagingSRV; };

private:
	void Create2DView(DX12Device* device, GFX_FORMAT_SET fmt, bool isCubemap = false);

private:
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_RTV;
	DX12DescriptorHandle m_StagingSRV;
};

