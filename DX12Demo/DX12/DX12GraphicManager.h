#pragma once

#include "../Utils/RingBufferAllocator.h"
#include "DX12Constants.h"
#include "DX12DescriptorHandle.h"

class DX12Device;
class DX12GpuResource;
class DX12Texture;
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

	ID3D12CommandQueue* GetSwapChainCommandQueue() const { return m_SwapChainCommandQueue.Get(); }

	void Flip();

#ifdef _XBOX_ONE
	void Suspend();
	void Resume();
#endif

	void CreateGraphicCommandQueues(uint32_t cnt = 1);

	// graphic context execution
	DX12GraphicContext* BegineGraphicContext();
	void EndGraphicContext(DX12GraphicContext* ctx);
	void ExecuteGraphicContext(DX12GraphicContext* ctx);
	void ExecuteGraphicContextInQueue(DX12GraphicContext* ctx, ID3D12CommandQueue* pQueue);

	// resource binding
	DX12DescriptorHandle RegisterResourceInDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

	void UpdateBufer(DX12GraphicContext* pGfxContext, DX12GpuResource* pResource, void* pSrcData, uint64_t sizeInBytes);

	void UpdateTexture(DX12GraphicContext* pGfxContext, DX12Texture* pTexture, uint32_t subresource, void* pSrcData, uint64_t sizeInBytes);

	void AllocateConstantsBuffer(uint32_t sizeInBytes, uint32_t alignInBytes, void** pCpuWrittablePtr, D3D12_GPU_VIRTUAL_ADDRESS* pGpuVirtualAddress);

private:
	DX12GraphicManager();
	~DX12GraphicManager();

	std::unique_ptr<DX12Device> m_Device;
	std::vector<ComPtr<ID3D12CommandQueue>> m_GraphicQueues;
	ComPtr<ID3D12CommandQueue> m_SwapChainCommandQueue;

	uint32_t m_GraphicContextParallelBits;

	uint32_t m_GraphicContextIdx;
	std::array<std::shared_ptr<DX12GraphicContext>, DX12NumGraphicContexts> m_GraphicContexts;

	std::unique_ptr<DX12DescriptorManager> m_DescriptorManager;

	std::unique_ptr<DX12FenceManager> m_FenceManager;

	ComPtr<ID3D12Heap> m_ConstantsBufferHeap;
	ComPtr<ID3D12Resource> m_ConstantsBuffer;
	RingBufferAllocator m_ConstantsBufferAllocator;
	uint8_t* m_ConstantsBufferBeginPtr;

	ComPtr<ID3D12Heap> m_UploadHeap;
	RingBufferAllocator m_UploadHeapAllocator;

	// timing wheel for temp resource lifetime management
	enum
	{
		NumTempResourcesPool = 4,
	};
	int32_t m_TempResourcePoolIdx;
	std::vector<ComPtr<ID3D12Resource>> m_TempResources[NumTempResourcesPool];

private:
	static DX12GraphicManager* s_GfxManager;
};
