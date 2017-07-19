#include "pch.h"
#include "DX12ColorSurface.h"

#include "DX12Device.h"
#include "DX12GraphicsManager.h"


DX12ColorSurface::DX12ColorSurface()
{
}

DX12ColorSurface::~DX12ColorSurface()
{
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, const RenderableSurfaceDesc& desc)
{
	uint32_t arraySize = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = desc.m_Format.RTVFormat;
	optimizedClearValue.Color[0] = 0.0f;
	optimizedClearValue.Color[1] = 0.0f;
	optimizedClearValue.Color[2] = 0.0f;
	optimizedClearValue.Color[3] = 0.0f;
	Init(device, desc.m_Format.BaseFormat, desc.m_Width, desc.m_Height, desc.m_ArraySize, desc.m_MipLevels, desc.m_SampleCount, sampleQuality, flags, &optimizedClearValue, initialState);

	Create2DView(device, desc.m_Format);
}

void DX12ColorSurface::InitAs2dSurface(DX12Device* device, ComPtr<ID3D12Resource> pResource, GFX_FORMAT_SET fmt, D3D12_RESOURCE_STATES initialState)
{
	SetGpuResource(pResource, initialState);

	Create2DView(device, fmt);
}

void DX12ColorSurface::Create2DView(DX12Device* device, GFX_FORMAT_SET fmt)
{
	D3D12_RESOURCE_DESC resourceDesc = GetGpuResource()->GetDesc();
	assert(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

	bool isSRVFormatValid = (fmt.SRVFormat !=  DXGI_FORMAT_UNKNOWN);
	bool isUAVFormatValid = (fmt.UAVFormat !=  DXGI_FORMAT_UNKNOWN);
	bool isRTVFormatValid = (fmt.RTVFormat !=  DXGI_FORMAT_UNKNOWN);

	if (isSRVFormatValid)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(CD3DX12::GetSRVDimension(resourceDesc), fmt.SRVFormat);

		m_SRV = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_SRV.GetCpuHandle());

		m_StagingSRV = DX12GraphicsManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_StagingSRV.GetCpuHandle());
	}

	if (isUAVFormatValid)
	{
		CD3DX12_UNORDERED_ACCESS_VIEW_DESC uavDesc = CD3DX12_UNORDERED_ACCESS_VIEW_DESC::Tex2DView(CD3DX12::GetUAVDimension(resourceDesc), fmt.UAVFormat);

		if (resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
		{
			m_UAV = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			device->CreateUnorderedAccessView(GetGpuResource(), &uavDesc, m_UAV.GetCpuHandle());

			m_StagingUAV = DX12GraphicsManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			device->CreateUnorderedAccessView(GetGpuResource(), &uavDesc, m_StagingUAV.GetCpuHandle());
		}
	}

	assert(isRTVFormatValid);
	CD3DX12_RENDER_TARGET_VIEW_DESC rtvDesc = CD3DX12_RENDER_TARGET_VIEW_DESC::Tex2DView(CD3DX12::GetRTVDimension(resourceDesc), fmt.RTVFormat);
	m_RTV = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(GetGpuResource(), &rtvDesc, m_RTV.GetCpuHandle());
}
