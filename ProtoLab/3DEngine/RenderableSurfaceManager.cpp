#include "pch.h"
#include "RenderableSurfaceManager.h"

#include "../DX12/Hash.h"


static RenderableSurfaceManager* s_RenderTargetManager;

void RenderableSurfaceManager::Initialize()
{
	s_RenderTargetManager = new RenderableSurfaceManager;
}

void RenderableSurfaceManager::Finalize()
{
	delete s_RenderTargetManager;
	s_RenderTargetManager = nullptr;
}

RenderableSurfaceManager* RenderableSurfaceManager::GetInstance()
{
	return s_RenderTargetManager;
}

RenderableSurfaceManager::RenderableSurfaceManager()
{
	for (uint32_t i = 0; i < m_Surfaces.size(); ++i)
	{
		m_Surfaces[i].m_Hash = 0;
		m_Surfaces[i].m_State = RS_Empty;
	}
}

RenderableSurfaceManager::~RenderableSurfaceManager()
{
}

RenderableSurfaceHandle<DX12ColorSurface> RenderableSurfaceManager::AcquireColorSurface(const RenderableSurfaceDesc & desc)
{
	return AcquireRenderableSurface<DX12ColorSurface>(desc);
}

RenderableSurfaceHandle<DX12DepthSurface> RenderableSurfaceManager::AcquireDepthSurface(const RenderableSurfaceDesc & desc)
{
	return AcquireRenderableSurface<DX12DepthSurface>(desc);
}

template<typename T>
void RenderableSurfaceManager::ReleaseRenderableSurface(const RenderableSurfaceHandle<T>& handle)
{
	assert(handle.isValid());
	assert(handle.m_Handle < m_Surfaces.size());

	RenderableSurfaceItem& item = m_Surfaces[handle.m_Handle];
	assert(handle.m_Hash == item.m_Hash);
	item.m_State = RS_Free;
}

template<typename T>
T* RenderableSurfaceManager::GetRenderableSurface(const RenderableSurfaceHandle<T>& handle)
{
	assert(handle.isValid());
	assert(handle.m_Handle < m_Surfaces.size());

	RenderableSurfaceItem& item = m_Surfaces[handle.m_Handle];
	assert(handle.m_Hash == item.m_Hash);
	assert(item.m_State == RS_Used);

	DX12RenderableSurface* surf = item.m_Surface.get();
	return dynamic_cast<T*>(surf);
}

template<typename T>
RenderableSurfaceHandle<T> RenderableSurfaceManager::AcquireRenderableSurface(const RenderableSurfaceDesc & desc)
{
	RenderableSurfaceHandle<T> handle;
	handle.m_Hash = 0;
	handle.m_Handle = (uint64_t)-1;

	uint64_t hashVal = Utility::HashState(&desc);

	for (uint32_t i = 0; i < m_Surfaces.size(); ++i)
	{
		if (m_Surfaces[i].m_Hash == hashVal && m_Surfaces[i].m_State == RS_Free)
		{
			// mark as used
			m_Surfaces[i].m_State = RS_Used;

			handle.m_Hash = hashVal;
			handle.m_Handle = i;
			break;
		}
	}

	for (uint32_t i = 0; i < m_Surfaces.size(); ++i)
	{
		if (m_Surfaces[i].m_State == RS_Empty)
		{
			// add a new surface and mark as used
			std::shared_ptr<T> surf = std::make_shared<T>();
			surf->InitAs2dSurface(DX12GraphicsManager::GetInstance()->GetDevice(), desc);
			m_Surfaces[i].m_Hash = hashVal;
			m_Surfaces[i].m_State = RS_Used;
			m_Surfaces[i].m_Surface = surf;

			handle.m_Hash = hashVal;
			handle.m_Handle = i;
			break;
		}
	}

	return handle;
}

template DX12ColorSurface* RenderableSurfaceManager::GetRenderableSurface<DX12ColorSurface>(const RenderableSurfaceHandle<DX12ColorSurface>& handle);
template DX12DepthSurface* RenderableSurfaceManager::GetRenderableSurface<DX12DepthSurface>(const RenderableSurfaceHandle<DX12DepthSurface>& handle);

template RenderableSurfaceHandle<DX12ColorSurface> RenderableSurfaceManager::AcquireRenderableSurface<DX12ColorSurface>(const RenderableSurfaceDesc & desc);
template RenderableSurfaceHandle<DX12DepthSurface> RenderableSurfaceManager::AcquireRenderableSurface<DX12DepthSurface>(const RenderableSurfaceDesc & desc);

template void RenderableSurfaceManager::ReleaseRenderableSurface<DX12ColorSurface>(const RenderableSurfaceHandle<DX12ColorSurface>& handle);
template void RenderableSurfaceManager::ReleaseRenderableSurface<DX12DepthSurface>(const RenderableSurfaceHandle<DX12DepthSurface>& handle);
