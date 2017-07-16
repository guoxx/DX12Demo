#pragma once

#include "DX12Wrapper.h"
#include "DX12RenderableSurface.h"
#include "DX12DescriptorHandle.h"

class DX12ColorSurface : public DX12RenderableSurface
{
public:
	DX12ColorSurface();
	virtual ~DX12ColorSurface();

	void InitAs2dSurface(DX12Device* device, const RenderableSurfaceDesc& desc);
	void InitAs2dSurface(DX12Device* device, ComPtr<ID3D12Resource> pResource, GFX_FORMAT_SET fmt, D3D12_RESOURCE_STATES initialtate);

	DX12DescriptorHandle GetSRV() const { return m_SRV; };
	DX12DescriptorHandle GetUAV() const { return m_UAV; };
	DX12DescriptorHandle GetRTV() const { return m_RTV; };

	DX12DescriptorHandle GetStagingSRV() const { return m_StagingSRV; };
	DX12DescriptorHandle GetStagingUAV() const { return m_StagingUAV; };

private:
	void Create2DView(DX12Device* device, GFX_FORMAT_SET fmt);

private:
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_UAV;
	DX12DescriptorHandle m_RTV;

	DX12DescriptorHandle m_StagingSRV;
	DX12DescriptorHandle m_StagingUAV;
};

