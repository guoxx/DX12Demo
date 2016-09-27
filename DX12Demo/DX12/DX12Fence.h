#pragma once

#include "DX12Constants.h"
#include "DX12Device.h"

class DX12Device;

class DX12Fence
{
	using Event = Microsoft::WRL::Wrappers::Event;

public:
	DX12Fence()
	{
	}

	~DX12Fence()
	{
		m_FenceEvent.Detach();
	}

	void Init(DX12Device* device)
	{
		m_Fence = device->CreateFence(0);
		m_FenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));
	}

	bool IsBusy() const
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

	void WaitForFence() const
	{
		WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);
	}

	void SignalFence(ID3D12CommandQueue* pCommandQueue, uint32_t newFenceValue)
	{
		assert(m_Fence->GetCompletedValue() < newFenceValue);

		// Schedule a Signal command in the GPU queue.
		DX::ThrowIfFailed(pCommandQueue->Signal(m_Fence.Get(), newFenceValue));

		// Setup fence event.
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(newFenceValue, m_FenceEvent.Get()));
	}

private:
	ComPtr<ID3D12Fence>		m_Fence;
	Event					m_FenceEvent;
};
