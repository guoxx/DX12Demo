#pragma once

#include "DX12Constants.h"

class DX12Device;
class DX12CopyContext;
class DX12GraphicContext;
class DX12DescriptorManager;
class DX12FenceManager;

class DX12GraphicManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicManager* GetInstance() { return s_GfxManager; }

	DX12FenceManager* GetFenceManager() const { return m_FenceManager.get(); }

	void CreateGraphicCommandQueues(uint32_t cnt = 1);

	DX12GraphicContext* BegineGraphicContext();
	void EndGraphicContext(DX12GraphicContext* ctx);
	void ExecuteGraphicContext(DX12GraphicContext* ctx);

	D3D12_CPU_DESCRIPTOR_HANDLE RegisterResourceInDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;
	std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicQueues;

	uint32_t m_GraphicContextIdx;
	std::array<std::shared_ptr<DX12GraphicContext>, DX12NumGraphicContexts> m_GraphicContexts;

	std::unique_ptr<DX12DescriptorManager> m_DescriptorManager;

	std::unique_ptr<DX12FenceManager> m_FenceManager;

private:
	static DX12GraphicManager* s_GfxManager;
};
