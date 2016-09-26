#pragma once

#include "DX12Fence.h"

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	virtual ~DX12CommandContext();

	virtual void Reset(DX12FenceHandle fenceHandle);

	void Close();

	void ClearState();

	bool IsBusy();

	void WaitForGPU();

	ID3D12GraphicsCommandList* GetCommandList() { return m_CommandList.Get(); }

protected:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

private:

	DX12FenceHandle m_FenceHandle;
};

