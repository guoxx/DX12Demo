#pragma once

#include "DX12Constants.h"

class DX12Device;

class DX12Fence
{
	using Event = Microsoft::WRL::Wrappers::Event;

public:
	DX12Fence();
	~DX12Fence();

	void Init(DX12Device* device);

	bool IsBusy()
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

	void WaitForFence()
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

class DX12FenceHandle
{
public:
	DX12FenceHandle();
	DX12FenceHandle(uint32_t idx);
	~DX12FenceHandle() = default;

	DX12Fence* GetFence();

private:
	uint32_t m_FenceIdx;
};

class DX12FenceManager
{
public:
	DX12FenceManager(DX12Device* device);
	~DX12FenceManager();

	void AdvanceFenceValue();

	void AdvanceSegment();

	DX12FenceHandle GetFenceHandle();

	DX12Fence* GetFence(uint32_t idx);

private:
	uint32_t m_CurrentFenceValue;
	uint32_t m_CurrentSegment;
	std::array<DX12Fence, DX12MaxFences> m_Fences;
};