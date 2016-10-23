#pragma once

#include "../DX12/DX12.h"


struct RenderableSurfaceDesc
{
	RenderableSurfaceDesc(GFX_FORMAT_SET format, uint32_t width, uint32_t height)
		: m_Format{ format }
		, m_Width{ width }
		, m_Height{ height }
	{
	}

	~RenderableSurfaceDesc() = default;

	GFX_FORMAT_SET m_Format;
	uint32_t m_Width;
	uint32_t m_Height;
};


struct RenderableSurfaceHandle
{
	friend class RenderableSurfaceManager;

public:
	RenderableSurfaceHandle() = default;
	~RenderableSurfaceHandle() = default;

	bool isValid() const
	{
		return m_Hash != 0 && m_Handle != -1;
	}

private:
	uint64_t m_Hash;
	uint64_t m_Handle;
};


class RenderableSurfaceManager
{
public:
	RenderableSurfaceManager();
	~RenderableSurfaceManager();

	static void Initialize();

	static void Finalize();

	static RenderableSurfaceManager* GetInstance();

	RenderableSurfaceHandle AcquireColorSurface(const RenderableSurfaceDesc& desc);
	RenderableSurfaceHandle AcquireDepthSurface(const RenderableSurfaceDesc& desc);

	DX12ColorSurface* GetColorSurface(const RenderableSurfaceHandle& handle);
	DX12DepthSurface* GetDepthSurface(const RenderableSurfaceHandle& handle);

	void ReleaseRenderableSurface(const RenderableSurfaceHandle& handle);

private:
	template<typename T>
	T* GetRenderableSurface(const RenderableSurfaceHandle& handle);

	template<typename T>
	RenderableSurfaceHandle AcquireRenderableSurface(const RenderableSurfaceDesc& desc);	

	enum
	{
		MaxRenderableSurfaces = 64,
	};
	
	enum RenderableSurfaceState
	{
		RS_Empty = 0,
		RS_Free = 1,
		RS_Used = 2,
	};

	struct RenderableSurfaceItem
	{
		uint64_t m_Hash;
		RenderableSurfaceState m_State;
		std::shared_ptr<DX12RenderableSurface> m_Surface;
	};

	std::array<RenderableSurfaceItem, MaxRenderableSurfaces> m_Surfaces;
};

