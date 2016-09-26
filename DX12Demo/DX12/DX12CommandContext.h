#pragma once

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	virtual ~DX12CommandContext();

	virtual void Reset();

	void Close();

	void ClearState();

	bool IsBusy();

	void WaitForGPU();

	void SignalFence(ID3D12CommandQueue* pCommandQueue);

	ID3D12GraphicsCommandList* GetCommandList() { return m_CommandList.Get(); }

protected:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

private:

	uint64_t m_FenceValue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	Microsoft::WRL::Wrappers::Event m_FenceEvent;
};

