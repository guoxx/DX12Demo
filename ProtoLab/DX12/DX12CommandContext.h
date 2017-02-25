#pragma once

#include "DX12FenceManager.h"

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	virtual ~DX12CommandContext();

	void SetParallelId(int32_t pid) { m_ParallelId = pid; }

	int32_t GetParallelId() const { return m_ParallelId; }

	bool IsBusy() const;

	void WaitForGPU() const;

	virtual void Reset();

	void Close();

	virtual void ExecuteInQueue(ID3D12CommandQueue* pCommandQueue);

protected:
	void ClearState();

	int32_t m_ParallelId;
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

private:

	DX12FenceHandle m_FenceHandle;
};

