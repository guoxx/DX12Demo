#pragma once

#include "DX12RenderableSurface.h"
#include "DX12DescriptorHandle.h"

class DX12ColorSurface : public DX12RenderableSurface
{
public:
	DX12ColorSurface();
	virtual ~DX12ColorSurface();

	void InitAs2dSurface(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height);
	void InitAs2dSurface(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height, uint32_t mipLevels);

	DX12DescriptorHandle GetSRV() const { return m_SRV; };
	DX12DescriptorHandle GetRTV() const { return m_RTV; };

private:
	void CreateView(DX12Device* device);

private:
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_RTV;
};

