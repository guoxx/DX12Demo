#pragma once

#include "DX12GraphicsContext.h"
#include "DX12GraphicsManager.h"

class DX12ScopedGraphicsContext : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12ScopedGraphicsContext()
	{
	    m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext();
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

class DX12ScopedSwapChainContext : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12ScopedSwapChainContext()
	{
	    m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext();
	}

	~DX12ScopedSwapChainContext()
	{
	    DX12GraphicsManager::GetInstance()->EndGraphicsContext(m_Ctx);

	    ID3D12CommandQueue* pQueue = DX12GraphicsManager::GetInstance()->GetSwapChainCommandQueue();
	    DX12GraphicsManager::GetInstance()->ExecuteGraphicsContextInQueue(m_Ctx, pQueue);
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