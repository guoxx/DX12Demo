#include "pch.h"
#include "DX12DepthSurface.h"

#include "DX12Wrapper.h"
#include "DX12Device.h"
#include "DX12GraphicManager.h"


DX12DepthSurface::DX12DepthSurface()
{
}

DX12DepthSurface::~DX12DepthSurface()
{
}

void DX12DepthSurface::InitAs2dSurface(DX12Device * device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height)
{
	InitAs2dSurface(device, fmt, width, height, 1);
}

void DX12DepthSurface::InitAs2dSurface(DX12Device * device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	uint32_t arraySize = 1;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = fmt.RTVFormat;
	optimizedClearValue.DepthStencil.Depth = 1.0f;
	optimizedClearValue.DepthStencil.Stencil = 0;
	Init(device, fmt.BaseFormat, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, &optimizedClearValue, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	CreateView(device, fmt);
}

void DX12DepthSurface::CreateView(DX12Device * device, GFX_FORMAT_SET fmt)
{
	D3D12_RESOURCE_DESC resourceDesc = GetGpuResource()->GetDesc();
	assert(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

	CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(CD3DX12::GetSRVDimension(resourceDesc), fmt.SRVFormat);
	CD3DX12_DEPTH_STENCIL_VIEW_DESC dsvDesc = CD3DX12_DEPTH_STENCIL_VIEW_DESC::Tex2DView(CD3DX12::GetDSVDimension(resourceDesc), fmt.DSVFormat);

	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_SRV.GetCpuHandle());

	m_DSV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(GetGpuResource(), &dsvDesc, m_DSV.GetCpuHandle());

	m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_StagingSRV.GetCpuHandle());
}
