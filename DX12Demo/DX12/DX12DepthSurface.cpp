#include "pch.h"
#include "DX12DepthSurface.h"

#include "DX12Device.h"
#include "DX12GraphicManager.h"


DX12DepthSurface::DX12DepthSurface()
{
}

DX12DepthSurface::~DX12DepthSurface()
{
}

void DX12DepthSurface::InitAs2dSurface(DX12Device * device, DXGI_FORMAT fmt, uint32_t width, uint32_t height)
{
	InitAs2dSurface(device, fmt, width, height, 1);
}

void DX12DepthSurface::InitAs2dSurface(DX12Device * device, DXGI_FORMAT fmt, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	uint32_t arraySize = 1;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//D3D12_CLEAR_VALUE optimizedClearValue;
	//optimizedClearValue.Format = fmt;
	//optimizedClearValue.DepthStencil.Depth = 1.0f;
	//optimizedClearValue.DepthStencil.Stencil = 0;
	Init(device, fmt, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, nullptr, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	CreateView(device);
}

void DX12DepthSurface::CreateView(DX12Device * device)
{
	// TODO: remove hardcode desc
	D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc;
	srvdesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvdesc.Texture2D.MipLevels = 1;
	srvdesc.Texture2D.MostDetailedMip = 0;
	srvdesc.Texture2D.PlaneSlice = 0;
	srvdesc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvdesc;
	dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvdesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvdesc.Texture2D.MipSlice = 0;


	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvdesc, m_SRV.GetCpuHandle());

	m_DSV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateDepthStencilView(GetGpuResource(), &dsvdesc, m_DSV.GetCpuHandle());

	m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvdesc, m_StagingSRV.GetCpuHandle());
}
