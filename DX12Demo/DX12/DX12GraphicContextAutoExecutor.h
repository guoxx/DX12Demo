#pragma once

#include "DX12GraphicContext.h"
#include "DX12GraphicManager.h"

class DX12GraphicContextAutoExecutor : public Heaponly, Noncopyable, Nonmovable
{
public:
	DX12GraphicContextAutoExecutor()
		: m_Ctx{ nullptr }
	{}

	~DX12GraphicContextAutoExecutor()
	{
		if (m_Ctx != nullptr)
		{
			DX12GraphicManager::GetInstance()->EndGraphicContext(m_Ctx);
			DX12GraphicManager::GetInstance()->ExecuteGraphicContext(m_Ctx);
		}
	}

	DX12GraphicContext* GetGraphicContext()
	{
		if (m_Ctx == nullptr)
		{
			m_Ctx = DX12GraphicManager::GetInstance()->BegineGraphicContext();
		}
		return m_Ctx;
	}

private:
	DX12GraphicContext*  m_Ctx;
};