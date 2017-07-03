#include "pch.h"
#include "DX12Texture.h"

#include "DX12Device.h"
#include "DX12GraphicsContext.h"
#include "DX12GraphicsManager.h"

#include "DirectXTex/DirectXTex.h"
#include <filesystem>

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

DX12Texture* DX12Texture::LoadFromFile(DX12Device* device, DX12GraphicsContext* pGfxContext, const char* filename, bool forceSRGB)
{
    std::experimental::filesystem::path filePath{filename};
    std::experimental::filesystem::path fileExt = filePath.extension();


    HRESULT result = S_OK;

    // load data
    TexMetadata metadata;
    ScratchImage scratchImg;
    if (fileExt == "dds")
    {
        result = LoadFromDDSFile(filePath.wstring().c_str(), DDS_FLAGS_NONE, &metadata, scratchImg);
        assert(result == S_OK);
    }
    else
    {
        ScratchImage rawScratchImg;
        result = LoadFromWICFile(filePath.wstring().c_str(), WIC_FLAGS_NONE, &metadata, rawScratchImg);
        assert(result == S_OK);

        result = GenerateMipMaps(*rawScratchImg.GetImage(0, 0, 0), TEX_FILTER_DEFAULT, 0, scratchImg, true);
        assert(result == S_OK);
    }

    // create d3d resource
	ComPtr<ID3D12Resource> d3dResoure;
    result = CreateTextureEx(device->GetD3DDevice(), metadata, D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS, forceSRGB, d3dResoure.ReleaseAndGetAddressOf());
    assert(result == S_OK);

    // prepare upload texture data description
	std::vector<D3D12_SUBRESOURCE_DATA> subesources;
    result = PrepareUpload(device->GetD3DDevice(), scratchImg.GetImages(), scratchImg.GetImageCount(), metadata, subesources);
    assert(result == S_OK);

    // create texture and upload resources
	DX12Texture* pTex = new DX12Texture(device, d3dResoure, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_COPY_DEST);
	pGfxContext->UploadGpuResource(pTex, 0, static_cast<uint32_t>(subesources.size()), subesources.data());
	pGfxContext->ResourceTransitionBarrier(pTex, D3D12_RESOURCE_STATE_GENERIC_READ);

	if (forceSRGB)
	{
		assert(DirectX::MakeSRGB(pTex->m_Format) == pTex->m_Format);
	}

	return pTex;
}

DX12Texture * DX12Texture::LoadFromBin(DX12Device * device, DX12GraphicsContext * pGfxContext, const uint8_t * pBinData,
	DXGI_FORMAT format, uint32_t width, uint32_t height)
{
	DX12Texture* pTex = new DX12Texture(device, format, width, height);

	size_t NumBytes = 0;
	size_t RowBytes = 0;
	DirectX::GetSurfaceInfo(width, height, format, &NumBytes, &RowBytes, nullptr);

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
	m_SRV = DX12GraphicsManager::GetInstance()->RegisterResourceInDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_SRV.GetCpuHandle());

	m_StagingSRV = DX12GraphicsManager::GetInstance()->RegisterResourceInStagingDescriptorHeap(GetGpuResource(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(GetGpuResource(), nullptr, m_StagingSRV.GetCpuHandle());
}
