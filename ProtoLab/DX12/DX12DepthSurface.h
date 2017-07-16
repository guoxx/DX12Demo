#pragma once

#include "DX12Wrapper.h"
#include "DX12RenderableSurface.h"
#include "DX12DescriptorHandle.h"

class DX12Device;

class DX12DepthSurface : public DX12RenderableSurface
{
public:
	DX12DepthSurface();
	virtual ~DX12DepthSurface();

	void InitAs2dSurface(DX12Device* device, const RenderableSurfaceDesc& desc);

	DX12DescriptorHandle GetSRV() const { return m_SRV; };
	DX12DescriptorHandle GetDSV() const { return m_DSV; };
	DX12DescriptorHandle GetStagingSRV() const { return m_StagingSRV; };

private:
	void Create2DView(DX12Device* device, GFX_FORMAT_SET fmt);

private:
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_DSV;
	DX12DescriptorHandle m_StagingSRV;
};

