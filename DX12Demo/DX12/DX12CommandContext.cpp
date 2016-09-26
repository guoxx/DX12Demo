#include "pch.h"
#include "DX12CommandContext.h"

#include "DX12Device.h"

DX12CommandContext::DX12CommandContext(DX12Device* device)
	: m_FenceHandle{ }
{
}

DX12CommandContext::~DX12CommandContext()
{
	assert(!IsBusy());
}

void DX12CommandContext::Reset(DX12FenceHandle fenceHandle)
{
	assert(!IsBusy());
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	m_FenceHandle = fenceHandle;
}

void DX12CommandContext::Close()
{
	m_CommandList->Close();
}

void DX12CommandContext::ClearState()
{
	m_CommandList->ClearState(nullptr);
}

bool DX12CommandContext::IsBusy()
{
	DX12Fence* fence = m_FenceHandle.GetFence();
	return fence != nullptr && fence->IsBusy();
}

void DX12CommandContext::WaitForGPU()
{
	DX12Fence* fence = m_FenceHandle.GetFence();
	if (fence != nullptr)
	{
		fence->WaitForFence();
	}
}
