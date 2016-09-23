#pragma once

class DX12Device;
class DX12CopyContext;
class DX12GraphicContext;

class DX12GraphicManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicManager* GetInstance() { return s_GfxManager; }

	void CreateCopyCommandQueues(uint32_t cnt);
	void CreateGraphicCommandQueues(uint32_t cnt);

	std::shared_ptr<DX12CopyContext> CreateCopyContext();
	std::shared_ptr<DX12GraphicContext> CreateGraphicContext();

	void ExecuteCopyContext(DX12CopyContext* ctx);
	void ExecuteGraphicContext(DX12GraphicContext* ctx);

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;
	std::vector<ComPtr<ID3D12CommandQueue>> m_CopyQueues;
	std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicQueues;

private:
	static DX12GraphicManager* s_GfxManager;
};

