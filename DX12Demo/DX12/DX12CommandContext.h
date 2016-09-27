#pragma once

#include "DX12FenceManager.h"

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	virtual ~DX12CommandContext();

	bool IsBusy() const;

	void WaitForGPU() const;

	virtual void Reset();

	void Close();

	void ExecuteInQueue(ID3D12CommandQueue* pCommandQueue);

protected:
	void ClearState();

	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

private:

	DX12FenceHandle m_FenceHandle;
};

