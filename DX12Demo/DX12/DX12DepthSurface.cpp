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
	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = fmt;
	optimizedClearValue.DepthStencil.Depth = 1.0f;
	optimizedClearValue.DepthStencil.Stencil = 0;
	Init(device, fmt, width, height, 1, mipLevels, &optimizedClearValue);

	CreateView(device);
}

void DX12DepthSurface::CreateView(DX12Device * device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SRV.GetCpuHandle());

	m_DSV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	device->CreateRenderTargetView(m_Resource.Get(), nullptr, m_DSV.GetCpuHandle());
}
