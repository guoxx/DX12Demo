#pragma once

class DX12Device;

class DX12CommandContext
{
public:
	DX12CommandContext(DX12Device* device);
	~DX12CommandContext();

	bool IsBusy();

	void WaitForGPU();

protected:
	ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;

private:
	void SignalFence(ID3D12CommandQueue* pCommandQueue);

	uint64_t m_FenceValue;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	Microsoft::WRL::Wrappers::Event m_FenceEvent;
};

