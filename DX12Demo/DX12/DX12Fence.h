#pragma once

#include "DX12Constants.h"
#include "DX12Device.h"

class DX12Device;

class DX12Fence
{
	using Event = Microsoft::WRL::Wrappers::Event;

public:
	DX12Fence()
		: m_IsSignalled{ false }
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
		if (!m_IsSignalled)
		{
			return false;
		}

		DWORD ret = WaitForSingleObjectEx(m_FenceEvent.Get(), IGNORE, true);
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

	void WaitForFence()
	{
		WaitForSingleObjectEx(m_FenceEvent.Get(), INFINITE, FALSE);

		assert(!IsBusy());
		 m_IsSignalled = false;
	}

	void SignalFence(ID3D12CommandQueue* pCommandQueue, uint32_t newFenceValue)
	{
		m_IsSignalled = true;

		assert(m_Fence->GetCompletedValue() < newFenceValue);

		// Schedule a Signal command in the GPU queue.
		DX::ThrowIfFailed(pCommandQueue->Signal(m_Fence.Get(), newFenceValue));

		// Setup fence event.
		DX::ThrowIfFailed(m_Fence->SetEventOnCompletion(newFenceValue, m_FenceEvent.Get()));
	}

private:
	bool					m_IsSignalled;
	ComPtr<ID3D12Fence>		m_Fence;
	Event					m_FenceEvent;
};
