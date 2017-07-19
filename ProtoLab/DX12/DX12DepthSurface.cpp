#include "pch.h"
#include "DX12DepthSurface.h"

#include "DX12Wrapper.h"
#include "DX12Device.h"
#include "DX12GraphicsManager.h"


DX12DepthSurface::DX12DepthSurface()
{
}

DX12DepthSurface::~DX12DepthSurface()
{
}

void DX12DepthSurface::InitAs2dSurface(DX12Device * device, const RenderableSurfaceDesc& desc)
{
	uint32_t arraySize = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	D3D12_CLEAR_VALUE optimizedClearValue;
	optimizedClearValue.Format = desc.m_Format.DSVFormat;
	optimizedClearValue.DepthStencil.Depth = 1.0f;
	optimizedClearValue.DepthStencil.Stencil = 0;
	Init(device, desc.m_Format.BaseFormat, desc.m_Width, desc.m_Height, desc.m_ArraySize, desc.m_MipLevels, desc.m_SampleCount, sampleQuality, flags, &optimizedClearValue, initialState);

	Create2DView(device, desc.m_Format);
}

void DX12DepthSurface::Create2DView(DX12Device * device, GFX_FORMAT_SET fmt)
{
	D3D12_RESOURCE_DESC resourceDesc = GetGpuResource()->GetDesc();
	assert(resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);

	CD3DX12_SHADER_RESOURCE_VIEW_DESC srvDesc = CD3DX12_SHADER_RESOURCE_VIEW_DESC::Tex2DView(CD3DX12::GetSRVDimension(resourceDesc), fmt.SRVFormat);

	m_SRV = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_SRV.GetCpuHandle());

    m_StagingSRV = DX12GraphicsManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    device->CreateShaderResourceView(GetGpuResource(), &srvDesc, m_StagingSRV.GetCpuHandle());

    for (int32_t arraySlice = 0; arraySlice < resourceDesc.DepthOrArraySize; ++arraySlice)
    {
        std::vector<DX12DescriptorHandle> dsvAry;
        for (int32_t mipLevel = 0; mipLevel < resourceDesc.MipLevels; ++mipLevel)
        {
            auto dsvDesc = CD3DX12_DEPTH_STENCIL_VIEW_DESC::Tex2DView(CD3DX12::GetDSVDimension(resourceDesc),
                                                                      fmt.DSVFormat,
                                                                      D3D12_DSV_FLAG_NONE,
                                                                      mipLevel,
                                                                      arraySlice);

            DX12DescriptorHandle handle = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            device->CreateDepthStencilView(GetGpuResource(), &dsvDesc, handle.GetCpuHandle());

            dsvAry.push_back(handle);
        }
        m_DSVs.push_back(dsvAry);
    }
}
