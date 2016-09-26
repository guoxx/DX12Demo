#pragma once

class DX12Device;
class DX12CopyContext;
class DX12GraphicContext;
class DX12DescriptorManager;

class DX12GraphicManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicManager* GetInstance() { return s_GfxManager; }

	void CreateGraphicCommandQueues(uint32_t cnt = 1);

	std::shared_ptr<DX12GraphicContext> CreateGraphicContext();

	void ExecuteGraphicContext(DX12GraphicContext* ctx);

	D3D12_CPU_DESCRIPTOR_HANDLE RegisterResourceInDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;
	std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicQueues;

	std::unique_ptr<DX12DescriptorManager> m_DescriptorManager;

private:
	static DX12GraphicManager* s_GfxManager;
};

