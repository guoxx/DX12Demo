#include "pch.h"
#include "DX12CommandContext.h"

#include "DX12Device.h"
#include "DX12GraphicsManager.h"

DX12CommandContext::DX12CommandContext(DX12Device* device)
	: m_FenceHandle{ }
	, m_ParallelId{ DX12ParallelIdInvalid }
{
}

DX12CommandContext::~DX12CommandContext()
{
	assert(!IsBusy());
}

bool DX12CommandContext::IsBusy() const
{
	return m_FenceHandle.IsBusy();
}

void DX12CommandContext::WaitForGPU() const
{
	m_FenceHandle.WaitForFence();
}

void DX12CommandContext::Reset()
{
	assert(!IsBusy());
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	ClearState();

	m_FenceHandle = DX12GraphicsManager::GetInstance()->GetFenceManager()->GetFenceHandle();
}

void DX12CommandContext::Close()
{
	m_CommandList->Close();
}

void DX12CommandContext::ExecuteInQueue(ID3D12CommandQueue* pCommandQueue)
{
	ID3D12CommandList* lists[] = { m_CommandList.Get() };
	pCommandQueue->ExecuteCommandLists(_countof(lists), lists);

	DX12GraphicsManager::GetInstance()->GetFenceManager()->SignalAndAdvance(pCommandQueue);
}

void DX12CommandContext::ClearState()
{
	m_CommandList->ClearState(nullptr);
}


