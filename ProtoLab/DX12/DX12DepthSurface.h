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
	DX12DescriptorHandle GetStagingSRV() const { return m_StagingSRV; };
	DX12DescriptorHandle GetDSV(int32_t arraySlice = 0, int32_t mipLevel = 0) const { return m_DSVs[arraySlice][mipLevel]; };

private:
	void Create2DView(DX12Device* device, GFX_FORMAT_SET fmt);

private:
	DX12DescriptorHandle m_SRV;
	DX12DescriptorHandle m_StagingSRV;
	std::vector<std::vector<DX12DescriptorHandle>> m_DSVs;
};

