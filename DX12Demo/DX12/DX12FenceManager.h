#pragma once

#include "DX12Fence.h"

class DX12FenceHandle
{
public:
	DX12FenceHandle();
	DX12FenceHandle(uint32_t idx);
	~DX12FenceHandle() = default;

	bool IsValid() const { return m_FenceIdx >= 0 && m_FenceIdx < DX12MaxFences; }

	bool IsBusy() const { return IsValid() && GetFence()->IsBusy(); }

	void WaitForFence() const { if (IsValid()) { GetFence()->WaitForFence(); } }

private:
	DX12Fence* GetFence() const;

	uint32_t m_FenceIdx;
};

class DX12FenceManager
{
public:
	DX12FenceManager(DX12Device* device);
	~DX12FenceManager();

	void AdvanceFenceValue();

	void AdvanceSegment();

	void SignalAndAdvance(ID3D12CommandQueue* pCommandQueue);

	DX12FenceHandle GetFenceHandle() const;

	DX12Fence* GetFence(uint32_t idx);

private:
	uint64_t m_CurrentFenceValue;
	uint32_t m_CurrentSegment;
	std::array<DX12Fence, DX12MaxFences> m_Fences;
};
