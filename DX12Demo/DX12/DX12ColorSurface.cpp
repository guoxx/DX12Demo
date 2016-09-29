#include "pch.h"
#include "DX12ColorSurface.h"

#include "DX12Device.h"
#include "DX12GraphicManager.h"


DX12ColorSurface::DX12ColorSurface()
{
}

DX12ColorSurface::~DX12ColorSurface()
{
}

void DX12ColorSurface::InitAs2dSurface(DX12Device * device, DXGI_FORMAT fmt, uint32_t width, uint32_t height)
{
	InitAs2dSurface(device, fmt, width, height, 1);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	uint32_t arraySize = 1;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = fmt;
	optimizedClearValue.Color[0] = 0.0f;
	optimizedClearValue.Color[1] = 0.0f;
	optimizedClearValue.Color[2] = 0.0f;
	optimizedClearValue.Color[3] = 0.0f;
	Init(device, fmt, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, &optimizedClearValue);

	CreateView(device);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, ID3D12Resource* pResource)
{
	m_Resource = pResource;
	CreateView(device);
}

void DX12ColorSurface::CreateView(DX12Device* device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SRV.GetCpuHandle());

	m_RTV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(m_Resource.Get(), nullptr, m_RTV.GetCpuHandle());
}
