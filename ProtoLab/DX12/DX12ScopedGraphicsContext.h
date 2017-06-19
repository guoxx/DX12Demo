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
