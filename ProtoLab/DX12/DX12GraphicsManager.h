#pragma once

#include "../Utils/RingBufferAllocator.h"
#include "DX12Constants.h"
#include "DX12DescriptorHandle.h"

class DX12Device;
class DX12GpuResource;
class DX12Texture;
class DX12GraphicsContext;
class DX12DescriptorManager;
class DX12FenceManager;

class DX12GraphicsManager
{
public:
	static void Initialize();
	static void Finalize();

	static DX12GraphicsManager* GetInstance() { return s_GfxManager; }

	DX12Device* GetDevice() const { return m_Device.get(); }

	DX12FenceManager* GetFenceManager() const { return m_FenceManager.get(); }

	DX12DescriptorManager* GetDescriptorManager() const { return m_DescriptorManager.get(); }

	ID3D12CommandQueue* GetGraphicsQueue() const { return m_GraphicsQueue.Get(); }

	void Flip();

#ifdef _XBOX_ONE
	void Suspend();
	void Resume();
#endif

	// graphic context execution
	DX12GraphicsContext* BegineGraphicsContext(const wchar_t* name);
	void EndGraphicsContext(DX12GraphicsContext* ctx);
	void ExecuteGraphicsContext(DX12GraphicsContext* ctx);

	// resource binding
	DX12DescriptorHandle RegisterResourceInDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

	DX12DescriptorHandle RegisterResourceInStagingDescriptorHeap(ID3D12Resource* resource, D3D12_DESCRIPTOR_HEAP_TYPE type);

	std::shared_ptr<DX12GpuResource> AllocateTempGpuResourceInUploadHeap(uint32_t sizeInBytes, uint32_t alignInBytes = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

	void AllocateConstantsBuffer(uint32_t sizeInBytes, uint32_t alignInBytes, void** pCpuWrittablePtr, D3D12_GPU_VIRTUAL_ADDRESS* pGpuVirtualAddress);

private:
	DX12GraphicsManager();
	~DX12GraphicsManager();

	void CreateGraphicCommandQueues();

	std::unique_ptr<DX12Device> m_Device;
	ComPtr<ID3D12CommandQueue> m_GraphicsQueue;

	uint32_t m_GraphicContextParallelBits;

	uint32_t m_GraphicContextIdx;
	std::array<std::shared_ptr<DX12GraphicsContext>, DX12NumGraphicContexts> m_GraphicContexts;

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
	std::vector<std::shared_ptr<DX12GpuResource>> m_TempResources[NumTempResourcesPool];

private:
	static DX12GraphicsManager* s_GfxManager;
};
