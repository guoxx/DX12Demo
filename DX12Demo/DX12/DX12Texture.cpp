#include "pch.h"
#include "DX12Texture.h"

#include "DX12Device.h"
#include "DX12GraphicManager.h"


DX12Texture::DX12Texture(DX12Device* device, DXGI_FORMAT fmt, uint32_t width, uint32_t height)
	: m_Format{ fmt }
	, m_Width{ width }
	, m_Height{ height }
	, m_MipLevels{ 1 }
{
	uint32_t arraySize = 1;
	uint32_t sampleCount = 1;
	uint32_t sampleQuality = 0;
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
	D3D12_TEXTURE_LAYOUT layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ;

	m_Resource = device->CreateCommittedTexture2DInDefaultHeap(m_Format, m_Width, m_Height, arraySize, m_MipLevels, sampleCount, sampleQuality, flags, layout, nullptr, initialState);
}

DX12Texture::~DX12Texture()
{
}

void DX12Texture::CreateView(DX12Device * device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SRV.GetCpuHandle());
}
