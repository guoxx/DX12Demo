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

void DX12ColorSurface::InitAs2dSurface(DX12Device * device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height)
{
	InitAs2dSurface(device, fmt, width, height, 1);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, GFX_FORMAT_SET fmt, uint32_t width, uint32_t height, uint32_t mipLevels)
{
	uint32_t arraySize = 1;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = fmt.RTVFormat;
	optimizedClearValue.Color[0] = 0.0f;
	optimizedClearValue.Color[1] = 0.0f;
	optimizedClearValue.Color[2] = 0.0f;
	optimizedClearValue.Color[3] = 0.0f;
	Init(device, fmt.BaseFormat, width, height, arraySize, mipLevels, sampleCount, sampleQuality, flags, &optimizedClearValue, initialState);

	Create2DView(device, fmt);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, ComPtr<ID3D12Resource> pResource, GFX_FORMAT_SET fmt, D3D12_RESOURCE_STATES usageState)
{
	SetGpuResource(pResource, usageState);

	Create2DView(device, fmt);
}

void DX12ColorSurface::InitAsCubeSurface(DX12Device * device, GFX_FORMAT_SET fmt, uint32_t size)
{
	InitAsCubeSurface(device, fmt, size, 1);
}

void DX12ColorSurface::InitAsCubeSurface(DX12Device * device, GFX_FORMAT_SET fmt, uint32_t size, uint32_t mipLevels)
{
	uint32_t arraySize = 6;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = fmt.RTVFormat;
	optimizedClearValue.Color[0] = 0.0f;
	optimizedClearValue.Color[1] = 0.0f;
	optimizedClearValue.Color[2] = 0.0f;
	optimizedClearValue.Color[3] = 0.0f;
	Init(device, fmt.BaseFormat, size, size, arraySize, mipLevels, sampleCount, sampleQuality, flags, &optimizedClearValue, initialState);

	Create2DView(device, fmt, true);
}

void DX12ColorSurface::Create2DView(DX12Device* device, GFX_FORMAT_SET fmt, bool isCubemap)
{
	D3D12_RESOURCE_DESC resourceDesc = GetGpuResource()->GetDesc();
	assert(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

	CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	if (isCubemap)
	{
		srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::TexCubeView(CD3DX12::GetSRVDimension(resourceDesc, isCubemap), fmt.SRVFormat);
	}
	else
	{
		srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(CD3DX12::GetSRVDimension(resourceDesc, isCubemap), fmt.SRVFormat);
	}
	CD3DX12_RENDER_TARGET_VIEW_DESC rtvDesc = CD3DX12_RENDER_TARGET_VIEW_DESC::Tex2DView(CD3DX12::GetRTVDimension(resourceDesc), fmt.RTVFormat);

	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_SRV.GetCpuHandle());

	m_RTV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GetGpuResource(), &rtvDesc, m_RTV.GetCpuHandle());

	m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_StagingSRV.GetCpuHandle());
}
