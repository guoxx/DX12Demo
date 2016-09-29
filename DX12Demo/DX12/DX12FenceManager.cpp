#include "pch.h"
#include "DX12FenceManager.h"

#include "DX12GraphicManager.h"

DX12FenceHandle::DX12FenceHandle()
	: m_FenceIdx((uint32_t)-1)
{
}

DX12FenceHandle::DX12FenceHandle(uint32_t idx)
	: m_FenceIdx(idx)
{
}

DX12Fence* DX12FenceHandle::GetFence() const
{
	assert(m_FenceIdx >= 0 && m_FenceIdx < DX12MaxFences);
	return DX12GraphicManager::GetInstance()->GetFenceManager()->GetFence(m_FenceIdx);
}

DX12FenceManager::DX12FenceManager(DX12Device* device)
{
	m_CurrentFenceValue = 1;
	m_CurrentSegment = 0;

	for (uint32_t i = 0; i < m_Fences.size(); ++i)
	{
		m_Fences[i].Init(device);
	}
}

DX12FenceManager::~DX12FenceManager()
{
}

void DX12FenceManager::AdvanceFenceValue()
{
	m_CurrentFenceValue += 1;
}

void DX12FenceManager::AdvanceSegment()
{
	m_CurrentSegment = (m_CurrentSegment + 1) % DX12MaxFences;
}

void DX12FenceManager::SignalAndAdvance(ID3D12CommandQueue * pCommandQueue)
{
	m_Fences[m_CurrentSegment].SignalFence(pCommandQueue, m_CurrentFenceValue);
	AdvanceSegment();
	AdvanceFenceValue();
}

DX12FenceHandle DX12FenceManager::GetFenceHandle() const
{
	assert(!m_Fences[m_CurrentSegment].IsBusy());
	return DX12FenceHandle(m_CurrentSegment);
}

DX12Fence* DX12FenceManager::GetFence(uint32_t idx)
{
	return &m_Fences[idx];
}
