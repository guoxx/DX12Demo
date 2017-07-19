#pragma once

#include "DX12GraphicsContext.h"
#include "DX12GraphicsManager.h"

class DX12ScopedGraphicsContext : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12ScopedGraphicsContext()
	{
	    m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext(L"ScopedGraphicsContext");
	}

	DX12ScopedGraphicsContext(const wchar_t* name)
	{
	    m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext(name);
	}

	~DX12ScopedGraphicsContext()
	{
	    DX12GraphicsManager::GetInstance()->EndGraphicsContext(m_Ctx);
	    DX12GraphicsManager::GetInstance()->ExecuteGraphicsContext(m_Ctx);
	}

	DX12GraphicsContext& operator *() const
	{
		return *m_Ctx;
	}

	DX12GraphicsContext* operator ->() const
	{
		return m_Ctx;
	}

	DX12GraphicsContext* Get() const
	{
		return m_Ctx;
	}

private:
	DX12GraphicsContext*  m_Ctx;
};

class DX12ScopedGpuMarker: public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12ScopedGpuMarker(DX12ScopedGraphicsContext& a_Ctx, const wchar_t* evt)
        : m_Ctx{a_Ctx.Get()}
	{
        m_Ctx->PIXBeginEvent(evt);
	}

	DX12ScopedGpuMarker(DX12GraphicsContext* a_Ctx, const wchar_t* evt)
        : m_Ctx{a_Ctx}
	{
        m_Ctx->PIXBeginEvent(evt);
	}

	~DX12ScopedGpuMarker()
	{
        m_Ctx->PIXEndEvent();
	}

private:
	DX12GraphicsContext*  m_Ctx;
};

#define GPU_MARKER(a_Ctx, a_Event) DX12ScopedGpuMarker scopedGpuMarker_##a_Event{a_Ctx, L#a_Event};
#define GPU_MARKER_NAMED(a_Ctx, a_Name) DX12ScopedGpuMarker scopedGpuMarker_##a_Event{a_Ctx, a_Name};