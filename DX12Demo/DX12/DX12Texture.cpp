#include "pch.h"
#include "DX12Texture.h"

#include "DX12Device.h"
#include "DX12GraphicContext.h"
#include "DX12GraphicManager.h"

#include "Texture/LoaderHelpers.h"
#include "Texture/TextureLoaderTga.h"
#include "Texture/DDSTextureLoader.h"


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

	ComPtr<ID3D12Resource> res = device->CreateCommittedTexture2DInDefaultHeap(m_Format, m_Width, m_Height, arraySize, m_MipLevels, sampleCount, sampleQuality, flags, layout, nullptr, initialState);
	SetGpuResource(res, initialState);

	CreateView(device);
}

DX12Texture::DX12Texture(DX12Device* device, ComPtr<ID3D12Resource> texture, D3D12_RESOURCE_STATES initialState)
{
	D3D12_RESOURCE_DESC resoureDesc = texture->GetDesc();
	m_Format = resoureDesc.Format;
	m_Width = static_cast<uint32_t>(resoureDesc.Width);
	m_Height = resoureDesc.Height;
	m_MipLevels = resoureDesc.MipLevels;

	SetGpuResource(texture, initialState);

	CreateView(device);
}

DX12Texture::~DX12Texture()
{
}

DX12Texture* DX12Texture::LoadFromTGAFile(DX12Device* device, DX12GraphicContext* pGfxContext, const char * filename, bool sRGB)
{
	TextureLoaderTga texLoader{ };
	texLoader.LoadTga(filename);

	DX12Texture* pTex = new DX12Texture(device,
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
		texLoader.GetWidth(),
		texLoader.GetHeight());

	size_t NumBytes = 0;
	size_t RowBytes = 0;
	DirectX::LoaderHelpers::GetSurfaceInfo(texLoader.GetWidth(),
		texLoader.GetHeight(),
		sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM,
		&NumBytes,
		&RowBytes,
		nullptr);

	D3D12_SUBRESOURCE_DATA subesources;
	subesources.pData = texLoader.GetData();
	subesources.RowPitch = RowBytes;
	subesources.SlicePitch = NumBytes;

	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadGpuResource(pTex, 0, 1, &subesources);
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_GENERIC_READ);

	return pTex;
}

DX12Texture* DX12Texture::LoadFromDDSFile(DX12Device* device, DX12GraphicContext* pGfxContext, const char* filename, bool sRGB)
{
	ComPtr<ID3D12Resource> res;
	std::unique_ptr<uint8_t[]> ddsData;
	std::vector<D3D12_SUBRESOURCE_DATA> subesources;

	size_t maxsize = 0;
	D3D12_RESOURCE_FLAGS resFlags = D3D12_RESOURCE_FLAG_NONE;
	unsigned int loadFlags = sRGB ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT;

	DirectX::LoadDDSTextureFromFileEx(device, DX::UTF8StrToUTF16(filename).c_str(), maxsize, resFlags, loadFlags, res, ddsData, subesources);

	DX12Texture* pTex = new DX12Texture(device, res, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadGpuResource(pTex, 0, static_cast<uint32_t>(subesources.size()), subesources.data());
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_GENERIC_READ);

	if (sRGB)
	{
		assert(DirectX::LoaderHelpers::MakeSRGB(pTex->m_Format) == pTex->m_Format);
	}

	return pTex;
}

DX12Texture * DX12Texture::LoadFromBin(DX12Device * device, DX12GraphicContext * pGfxContext, const uint8_t * pBinData,
	DXGI_FORMAT format, uint32_t width, uint32_t height)
{
	DX12Texture* pTex = new DX12Texture(device, format, width, height);

	size_t NumBytes = 0;
	size_t RowBytes = 0;
	DirectX::LoaderHelpers::GetSurfaceInfo(width, height, format, &NumBytes, &RowBytes, nullptr);

	D3D12_SUBRESOURCE_DATA subesources;
	subesources.pData = pBinData;
	subesources.RowPitch = RowBytes;
	subesources.SlicePitch = NumBytes;

	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadGpuResource(pTex, 0, 1, &subesources);
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_GENERIC_READ);

	return pTex;
}

void DX12Texture::CreateView(DX12Device * device)
{
	m_SRV = DX12GraphicManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_SRV.GetCpuHandle());

	m_StagingSRV = DX12GraphicManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_StagingSRV.GetCpuHandle());
}
