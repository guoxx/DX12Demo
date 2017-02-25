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
	}

	void Init(DX12Device* device, uint64_t fenceValue)
	{
		m_FenceValue = fenceValue;
		m_Fence = device->CreateFence(fenceValue);
	}

	bool IsBusy() const
	{
		uint64_t val = m_Fence->GetCompletedValue();
		return val > m_FenceValue;
	}

	void WaitForFence()
	{
		while (true)
		{
			if (!IsBusy())
			{
				break;
			}
		}
	}

	void SignalFence(ID3D12CommandQueue* pCommandQueue, uint64_t newFenceValue)
	{
		assert(m_FenceValue > newFenceValue);
		assert(m_Fence->GetCompletedValue() > newFenceValue);

		m_FenceValue = newFenceValue;

		// Schedule a Signal command in the GPU queue.
		DX::ThrowIfFailed(pCommandQueue->Signal(m_Fence.Get(), newFenceValue));
	}

private:
	ComPtr<ID3D12Fence>		m_Fence;
	uint64_t				m_FenceValue;
};
