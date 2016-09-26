#include "pch.h"
#include "DX12CommandContext.h"

#include "DX12Device.h"

DX12CommandContext::DX12CommandContext(DX12Device* device)
{
	m_FenceValue = 1;
	m_Fence = device->CreateFence(0);
	m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
}

DX12CommandContext::~DX12CommandContext()
{
}

bool DX12CommandContext::IsBusy()
{
    DWORD ret = WaitForSingleObjectEx(m_FenceEvent.Get(), IGNORE, FALSE);
	if (ret == WAIT_OBJECT_0)
	{
		return false;
	}
	else
	{
		assert(ret == WAIT_TIMEOUT);
		return true;
	}
}

void DX12CommandContext::WaitForGPU()
{
    WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
}

void DX12CommandContext::SignalFence(ID3D12CommandQueue* pCommandQueue)
{
	assert(m_Fence->GetCompletedValue() < m_FenceValue);

    // Schedule a Signal command in the GPU queue.
    DX::ThrowIfFailed(pCommandQueue->Signal(m_Fence.Get(), m_FenceValue));

    // Setup fence event.
    DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent.Get()));

    // Increment the fence value
    m_FenceValue += 1;
}
