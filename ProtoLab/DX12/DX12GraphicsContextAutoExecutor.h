#pragma once

#include "DX12GraphicsContext.h"
#include "DX12GraphicsManager.h"

class DX12GraphicsContextAutoExecutor : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12GraphicsContextAutoExecutor()
		: m_Ctx{ nullptr }
	{}

	~DX12GraphicsContextAutoExecutor()
	{
		if (m_Ctx != nullptr)
		{
			DX12GraphicsManager::GetInstance()->EndGraphicsContext(m_Ctx);
			DX12GraphicsManager::GetInstance()->ExecuteGraphicsContext(m_Ctx);
		}
	}

	DX12GraphicsContext* GetGraphicsContext()
	{
		if (m_Ctx == nullptr)
		{
			m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext();
		}
		return m_Ctx;
	}

private:
	DX12GraphicsContext*  m_Ctx;
};

class DX12SwapChainContextAutoExecutor : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12SwapChainContextAutoExecutor()
		: m_Ctx{ nullptr }
	{}

	~DX12SwapChainContextAutoExecutor()
	{
		if (m_Ctx != nullptr)
		{
			DX12GraphicsManager::GetInstance()->EndGraphicsContext(m_Ctx);

			ID3D12CommandQueue* pQueue = DX12GraphicsManager::GetInstance()->GetSwapChainCommandQueue();
			DX12GraphicsManager::GetInstance()->ExecuteGraphicsContextInQueue(m_Ctx, pQueue);
		}
	}

	DX12GraphicsContext* GetGraphicsContext()
	{
		if (m_Ctx == nullptr)
		{
			m_Ctx = DX12GraphicsManager::GetInstance()->BegineGraphicsContext();
		}
		return m_Ctx;
	}

private:
	DX12GraphicsContext*  m_Ctx;
};