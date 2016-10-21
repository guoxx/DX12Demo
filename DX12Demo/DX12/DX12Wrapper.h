#pragma once

#ifdef _XBOX_ONE
#include "d3d12_x.h"
#include "d3dx12_x.h"
#else
#include "d3d12.h"
#include "d3dx12.h"
#endif

struct GFX_FORMAT_SET
{
	DXGI_FORMAT BaseFormat;
	union
	{
		DXGI_FORMAT RTVFormat;
		DXGI_FORMAT DSVFormat;
	};
	DXGI_FORMAT SRVFormat;
};

constexpr GFX_FORMAT_SET GFX_FORMAT_R8G8B8A8_UNORM = { DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM };
constexpr GFX_FORMAT_SET GFX_FORMAT_R8G8B8A8_UNORM_SRGB = { DXGI_FORMAT_R8G8B8A8_TYPELESS, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
constexpr GFX_FORMAT_SET GFX_FORMAT_D32_FLOAT = { DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT };
constexpr GFX_FORMAT_SET GFX_FORMAT_R32G32B32A32_FLOAT = { DXGI_FORMAT_R32G32B32A32_TYPELESS, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT };

struct CD3DX12_SHADER_RESOURCE_VIEW_DESC : public D3D12_SHADER_RESOURCE_VIEW_DESC
{
	CD3DX12_SHADER_RESOURCE_VIEW_DESC() = default;
	~CD3DX12_SHADER_RESOURCE_VIEW_DESC() = default;

    operator const CD3DX12_SHADER_RESOURCE_VIEW_DESC&() const { return *this; }

	explicit CD3DX12_SHADER_RESOURCE_VIEW_DESC(const D3D12_SHADER_RESOURCE_VIEW_DESC& o) :
		D3D12_SHADER_RESOURCE_VIEW_DESC(o)
	{}

	static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC BufferView(
		DXGI_FORMAT format,
		UINT firstElement,
		UINT numElements,
		UINT structureByteStride,
		D3D12_BUFFER_SRV_FLAGS flags = D3D12_BUFFER_SRV_FLAG_NONE)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Buffer.FirstElement = firstElement;
		desc.Buffer.NumElements = numElements;
		desc.Buffer.StructureByteStride = structureByteStride;
		desc.Buffer.Flags = flags;
		return desc;
	}

	static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC Tex1DView(
		D3D12_SRV_DIMENSION viewDimension,
		DXGI_FORMAT format,
		UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		UINT mostDetailedMip = 0,
		UINT mipLevels = -1,
		FLOAT resourceMinLODClamp = 0.0f,
		UINT firstArraySlice = 0,
		UINT arraySize = -1)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = viewDimension;
		desc.Shader4ComponentMapping = shader4ComponentMapping;

		switch (viewDimension)
		{
		case D3D12_SRV_DIMENSION_TEXTURE1D:
			desc.Texture1D.MostDetailedMip = mostDetailedMip;
			desc.Texture1D.MipLevels = mipLevels;
			desc.Texture1D.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
			desc.Texture1DArray.MostDetailedMip = mostDetailedMip;
			desc.Texture1DArray.MipLevels = mipLevels;
			desc.Texture1DArray.FirstArraySlice = firstArraySlice;
			desc.Texture1DArray.ArraySize = arraySize;
			desc.Texture1DArray.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		default:
			assert(false);
		}
		return desc;
	}

	static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC Tex2DView(
		D3D12_SRV_DIMENSION viewDimension,
		DXGI_FORMAT format,
		UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		UINT mostDetailedMip = 0,
		UINT mipLevels = -1,
		UINT planeSlice = 0,
		FLOAT resourceMinLODClamp = 0.0f,
		UINT firstArraySlice = 0,
		UINT arraySize = -1)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = viewDimension;
		desc.Shader4ComponentMapping = shader4ComponentMapping;

		switch (viewDimension)
		{
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			desc.Texture2D.MostDetailedMip = mostDetailedMip;
			desc.Texture2D.MipLevels = mipLevels;
			desc.Texture2D.PlaneSlice = planeSlice;
			desc.Texture2D.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
			desc.Texture2DArray.MostDetailedMip = mostDetailedMip;
			desc.Texture2DArray.MipLevels = mipLevels;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.PlaneSlice = planeSlice;
			desc.Texture2DArray.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMS:
			break;
		case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
			desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DMSArray.ArraySize = arraySize;
			break;
		default:
			assert(false);
		}
		return desc;
	}

	static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC TexCubeView(
		D3D12_SRV_DIMENSION viewDimension,
		DXGI_FORMAT format,
		UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		UINT mostDetailedMip = 0,
		UINT mipLevels = -1,
		FLOAT resourceMinLODClamp = 0.0f,
		UINT first2DArrayFace = 0,
		UINT numCubes = 1)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = viewDimension;
		desc.Shader4ComponentMapping = shader4ComponentMapping;

		switch (viewDimension)
		{
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			desc.TextureCube.MostDetailedMip = mostDetailedMip;
			desc.TextureCube.MipLevels = mipLevels;
			desc.TextureCube.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
			desc.TextureCubeArray.MostDetailedMip = mostDetailedMip;
			desc.TextureCubeArray.MipLevels = mipLevels;
			desc.TextureCubeArray.First2DArrayFace = first2DArrayFace;
			desc.TextureCubeArray.NumCubes = numCubes;
			desc.TextureCubeArray.ResourceMinLODClamp = resourceMinLODClamp;
			break;
		default:
			assert(false);
		}
		return desc;
	}

	static inline CD3DX12_SHADER_RESOURCE_VIEW_DESC Tex3DView(
		DXGI_FORMAT format,
		UINT shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		UINT mostDetailedMip = 0,
		UINT mipLevels = -1,
		FLOAT resourceMinLODClamp = 0.0f)
	{
		CD3DX12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		desc.Shader4ComponentMapping = shader4ComponentMapping;
		desc.Texture3D.MostDetailedMip = mostDetailedMip;
		desc.Texture3D.MipLevels = mipLevels;
		desc.Texture3D.ResourceMinLODClamp = resourceMinLODClamp;
		return desc;
	}
};

struct CD3DX12_RENDER_TARGET_VIEW_DESC : public D3D12_RENDER_TARGET_VIEW_DESC
{
	CD3DX12_RENDER_TARGET_VIEW_DESC() = default;
	~CD3DX12_RENDER_TARGET_VIEW_DESC() = default;

    operator const CD3DX12_RENDER_TARGET_VIEW_DESC&() const { return *this; }

	explicit CD3DX12_RENDER_TARGET_VIEW_DESC(const D3D12_RENDER_TARGET_VIEW_DESC& o) :
		D3D12_RENDER_TARGET_VIEW_DESC(o)
	{}

	static inline CD3DX12_RENDER_TARGET_VIEW_DESC Tex2DView(D3D12_RTV_DIMENSION viewDimension,
		DXGI_FORMAT format,
		UINT mipSlice = 0,
		UINT planeSlice = 0,
		UINT firstArraySlice = 0,
		UINT arraySize = -1)
	{
		CD3DX12_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = viewDimension;
		switch (viewDimension)
		{
		case D3D12_RTV_DIMENSION_TEXTURE2D:
			desc.Texture2D.MipSlice = mipSlice;
			desc.Texture2D.PlaneSlice = planeSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.MipSlice = mipSlice;
			desc.Texture2DArray.PlaneSlice = planeSlice;
			break;
		case D3D12_RTV_DIMENSION_TEXTURE2DMS:
			break;
		case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
			desc.Texture2DMSArray.ArraySize = arraySize;
			desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
			break;
		default:
			assert(false);
			break;
		}
		return desc;
	}
};

struct CD3DX12_DEPTH_STENCIL_VIEW_DESC : public D3D12_DEPTH_STENCIL_VIEW_DESC
{
	CD3DX12_DEPTH_STENCIL_VIEW_DESC() = default;
	~CD3DX12_DEPTH_STENCIL_VIEW_DESC() = default;

    operator const CD3DX12_DEPTH_STENCIL_VIEW_DESC&() const { return *this; }

	explicit CD3DX12_DEPTH_STENCIL_VIEW_DESC(const D3D12_DEPTH_STENCIL_VIEW_DESC& o) :
		D3D12_DEPTH_STENCIL_VIEW_DESC(o)
	{}

	static inline CD3DX12_DEPTH_STENCIL_VIEW_DESC Tex2DView(D3D12_DSV_DIMENSION viewDimension,
		DXGI_FORMAT format,
		D3D12_DSV_FLAGS flags = D3D12_DSV_FLAG_NONE,
		UINT mipSlice = 0,
		UINT firstArraySlice = 0,
		UINT arraySize = -1)
	{
		CD3DX12_DEPTH_STENCIL_VIEW_DESC desc;
		desc.Format = format;
		desc.ViewDimension = viewDimension;
		desc.Flags = flags;
		switch (viewDimension)
		{
		case D3D12_DSV_DIMENSION_TEXTURE2D:
			desc.Texture2D.MipSlice = mipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
			desc.Texture2DArray.ArraySize = arraySize;
			desc.Texture2DArray.FirstArraySlice = firstArraySlice;
			desc.Texture2DArray.MipSlice = mipSlice;
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DMS:
			break;
		case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
			desc.Texture2DMSArray.ArraySize = arraySize;
			desc.Texture2DMSArray.FirstArraySlice = firstArraySlice;
			break;
		default:
			assert(false);
			break;
		}
		return desc;
	}
};

namespace CD3DX12
{
	inline D3D12_SRV_DIMENSION GetSRVDimension(D3D12_RESOURCE_DESC& desc, bool isCube = false)
	{
		switch (desc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_BUFFER:
		{
			return D3D12_SRV_DIMENSION_BUFFER;
		}
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		{
			return desc.DepthOrArraySize > 1 ? D3D12_SRV_DIMENSION_TEXTURE1DARRAY : D3D12_SRV_DIMENSION_TEXTURE1D;
		}
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		{
			if (isCube)
			{
				assert(desc.SampleDesc.Count == 1);
				assert(desc.DepthOrArraySize == 6);
				return D3D12_SRV_DIMENSION_TEXTURECUBE;
			}
			else
			{
				if (desc.DepthOrArraySize > 1)
				{
					return desc.SampleDesc.Count > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				}
				else
				{
					return desc.SampleDesc.Count > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DMS : D3D12_SRV_DIMENSION_TEXTURE2D;
				}
			}
		}
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		{
			return D3D12_SRV_DIMENSION_TEXTURE3D;
		}
		default:
		{
			assert(false);
			return D3D12_SRV_DIMENSION_UNKNOWN;
		}
		}
	}

	inline D3D12_RTV_DIMENSION GetRTVDimension(D3D12_RESOURCE_DESC& desc)
	{
		D3D12_SRV_DIMENSION srvDimension = GetSRVDimension(desc, false);

		if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
			return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		}
		else if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBEARRAY)
		{
			assert(false);
			return D3D12_RTV_DIMENSION_UNKNOWN;
		}
		else
		{
			return (D3D12_RTV_DIMENSION)srvDimension;
		}
	}

	inline D3D12_DSV_DIMENSION GetDSVDimension(D3D12_RESOURCE_DESC& desc)
	{
		switch (desc.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
		{
			return desc.DepthOrArraySize > 1 ? D3D12_DSV_DIMENSION_TEXTURE1DARRAY : D3D12_DSV_DIMENSION_TEXTURE1D;
		}
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
		{
			if (desc.DepthOrArraySize > 1)
			{
				return desc.SampleDesc.Count > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			}
			else
			{
				return desc.SampleDesc.Count > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
			}
		}
		default:
			assert(false);
			return D3D12_DSV_DIMENSION_UNKNOWN;
		}
	}

	inline D3D12_UAV_DIMENSION GetUAVDimension(D3D12_RESOURCE_DESC& desc)
	{
		D3D12_SRV_DIMENSION srvDimension = GetSRVDimension(desc, false);
		if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
			return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		}
		else if (srvDimension == D3D12_SRV_DIMENSION_TEXTURECUBEARRAY)
		{
			assert(false);
			return D3D12_UAV_DIMENSION_UNKNOWN;
		}
		else if (srvDimension == D3D12_SRV_DIMENSION_TEXTURE2DMS || D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY)
		{
			assert(false);
			return D3D12_UAV_DIMENSION_UNKNOWN;
		}
		else
		{
			return (D3D12_UAV_DIMENSION)srvDimension;
		}
	}
}
