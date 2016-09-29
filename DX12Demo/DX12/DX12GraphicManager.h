#pragma once

#include "../Utils/RingBufferAllocator.h"
#include "DX12Constants.h"
#include "DX12DescriptorHandle.h"

class DX12Device;
class DX12GpuResource;
class DX12GraphicContext;
class DX12DescriptorManager;
class DX12FenceManager;

class DX12GraphicManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicManager* GetInstance() { return s_GfxManager; }

	DX12Device* GetDevice() const { return m_Device.get(); }

	DX12FenceManager* GetFenceManager() const { return m_FenceManager.get(); }

#ifdef _XBOX_ONE
	void Suspend();
	void Resume();
#endif

	void CreateGraphicCommandQueues(uint32_t cnt = 1);

	// graphic context execution
	DX12GraphicContext* BegineGraphicContext();
	void EndGraphicContext(DX12GraphicContext* ctx);
	void ExecuteGraphicContext(DX12GraphicContext* ctx);

	// resource binding
	DX12DescriptorHandle RegisterResourceInDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

	void UpdateBufer(DX12GraphicContext* pGfxContext, DX12GpuResource* pResource, void* pSrcData, uint64_t sizeInBytes);

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;
	std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicQueues;

	uint32_t m_GraphicContextIdx;
	std::array<std::shared_ptr<DX12GraphicContext>, DX12NumGraphicContexts> m_GraphicContexts;

	std::unique_ptr<DX12DescriptorManager> m_DescriptorManager;

	std::unique_ptr<DX12FenceManager> m_FenceManager;

	ComPtr<ID3D12Heap> m_UploadHeap;
	RingBufferAllocator m_UploadHeapAllocator;

	// TODO: using timing wheel to delete temp resources
	std::vector<ComPtr<ID3D12Resource>> m_TempResources;

private:
	static DX12GraphicManager* s_GfxManager;
};
