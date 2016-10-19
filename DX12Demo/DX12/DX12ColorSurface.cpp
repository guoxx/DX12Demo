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
	Init(device, fmt, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, &optimizedClearValue, D3D12_RESOURCE_STATE_RENDER_TARGET);

	CreateView(device);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, ComPtr<ID3D12Resource> pResource, D3D12_RESOURCE_STATES usageState)
{
	SetGpuResource(pResource, usageState);

	CreateView(device);
}

void DX12ColorSurface::CreateView(DX12Device* device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_SRV.GetCpuHandle());

	m_RTV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GetGpuResource(), nullptr, m_RTV.GetCpuHandle());

	m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_StagingSRV.GetCpuHandle());
}
