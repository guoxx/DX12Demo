#pragma once

#include "../DX12/DX12.h"

template <typename T>
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

	T* Get() const
	{
		return RenderableSurfaceManager::GetInstance()->GetRenderableSurface<T>(*this);
	}

	T& operator *() const
	{
		return *Get();
	}

	T* operator ->() const
	{
		return Get();
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

	RenderableSurfaceHandle<DX12ColorSurface> AcquireColorSurface(const RenderableSurfaceDesc& desc);
	RenderableSurfaceHandle<DX12DepthSurface> AcquireDepthSurface(const RenderableSurfaceDesc& desc);

	template<typename T>
	T* GetRenderableSurface(const RenderableSurfaceHandle<T>& handle);

	template<typename T>
	void ReleaseRenderableSurface(const RenderableSurfaceHandle<T>& handle);

private:

	template<typename T>
	RenderableSurfaceHandle<T> AcquireRenderableSurface(const RenderableSurfaceDesc& desc);	

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

