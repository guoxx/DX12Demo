#include "pch.h"
#include "DX12Texture.h"

#include "DX12Device.h"
#include "DX12GraphicContext.h"
#include "DX12GraphicManager.h"
#include "Texture/TextureLoaderTga.h"


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

DX12Texture* DX12Texture::LoadTGAFromFile(DX12Device* device, DX12GraphicContext* pGfxContext, const char * filename, bool sRGB)
{
	TextureLoaderTga texLoader{ };
	texLoader.LoadTga(filename);

	DX12Texture* pTex = new DX12Texture(device,
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
		texLoader.GetWidth(),
		texLoader.GetHeight());

	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST);
	DX12GraphicManager::GetInstance()->UpdateTexture(pGfxContext, pTex, 0, (uint8_t*)texLoader.GetData(), texLoader.GetDataSize());
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	return pTex;
}

void DX12Texture::CreateView(DX12Device * device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(m_Resource.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(m_Resource.Get(), nullptr, m_SRV.GetCpuHandle());
}
