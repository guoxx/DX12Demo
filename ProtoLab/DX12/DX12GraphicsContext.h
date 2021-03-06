#pragma once

#include "DX12CommandContext.h"


class DX12GpuResource;
class DX12VertexBuffer;
class DX12IndexBuffer;
class DX12ConstantsBuffer;
class DX12StructuredBuffer;
class DX12ColorSurface;
class DX12DepthSurface;
class DX12RootSignature;
class DX12PipelineState;
class DX12DescriptorHandle;

class DX12GraphicsContext : public DX12CommandContext
{
	using super = DX12CommandContext;

public:
	DX12GraphicsContext(DX12Device* device);
	virtual ~DX12GraphicsContext();

	void PIXBeginEvent(const wchar_t* label);

	void PIXEndEvent(void);

	void PIXSetMarker(const wchar_t* label);

    void SetName(const wchar_t* name);

	void IASetIndexBuffer(const DX12IndexBuffer* pIndexBuffer);

	void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);

	void ResolvePendingBarriers();

	void ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	void SetDescriptorHeaps(uint32_t numDescriptorHeaps, ID3D12DescriptorHeap** ppDescriptorHeaps);

	void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void *pSrcData, uint32_t destOffsetIn32BitValues);

	void SetGraphicsRootDynamicConstantBufferView(uint32_t rootParameterIndex, void* pData, uint32_t sizeInBytes);

	void SetGraphicsDynamicVertexBuffer(uint32_t slot, void* pData, uint32_t sizeInBytes, uint32_t strideInBytes);

	void SetGraphicsRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer* pStructuredBuffer);

	void SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle);

	void SetGraphicsDynamicCbvSrvUav(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);

	void SetComputeRootDynamicConstantBufferView(uint32_t rootParameterIndex, void* pData, uint32_t sizeInBytes);

	void SetComputeRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer);

	void SetComputeRootRWStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer);

	void SetComputeRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle);

	void SetComputeDynamicCbvSrvUav(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);

	void SetGraphicsRootSignature(std::shared_ptr<DX12RootSignature> pRootSig);

	void SetComputeRootSignature(std::shared_ptr<DX12RootSignature> pRootSig);

	void SetPipelineState(DX12PipelineState* pPSO);

	void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ);

	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY);

	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation = 0);

	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);

	void CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION *pDst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION *pSrc, const D3D12_BOX *pSrcBox);

	void UploadBuffer(DX12GpuResource* pResource, void* pSrcData, uint64_t sizeInBytes);

	void UploadGpuResource(DX12GpuResource* pDstResource, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* pSubData);

	void CopyResource(DX12GpuResource* srcResource, DX12GpuResource* dstResource);

	void ClearRenderTarget(DX12ColorSurface* pColorSurface, float r, float g, float b, float a);

	void ClearDepthTarget(DX12DepthSurface* pDepthSurface, float d);

	void SetRenderTarget(DX12ColorSurface* pColorSurface);

    void SetRenderTarget(DX12ColorSurface* pColorSurface, DX12DepthSurface* pDepthSurface);

	void SetRenderTargets(uint32_t numColorSurfaces, DX12ColorSurface* pColorSurface[], DX12DepthSurface* pDepthSurface);

	void SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height, float minDepth = 0.0f, float maxDepth = 1.0f);

	virtual void ExecuteInQueue(ID3D12CommandQueue* pCommandQueue) override final;

private:

	void RootSignatureChanged(std::shared_ptr<DX12RootSignature> pRootSig);

	void StageDynamicDescriptor(uint32_t rootParameterIndex, uint32_t offsetInTable, D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle);

	void ApplyDynamicDescriptors(bool bComputeCommand);

	struct PendingResourcBarrier
	{
		D3D12_RESOURCE_BARRIER m_Barrier;
		DX12GpuResource* m_DX12Resource;
	};

	bool m_FlushPendingBarriers;
	ComPtr<ID3D12GraphicsCommandList> m_BarriersCommandList;
	std::vector<PendingResourcBarrier> m_PendingTransitionBarriers;

	std::shared_ptr<DX12RootSignature> m_CurrentRootSig;

	enum
	{
		NumCachedHandlesPerNode = 32,
	};

	struct CpuDescriptorHandlesCache
	{
		int32_t											m_RootParamIndex{ -1 };
		uint32_t										m_TableSize{ 0 };

		uint32_t										m_TableStart;
		uint32_t										m_TableEnd;
		D3D12_CPU_DESCRIPTOR_HANDLE						m_CachedHandles[NumCachedHandlesPerNode];

		std::shared_ptr<CpuDescriptorHandlesCache>		m_Next;

		uint32_t GetTableEnd()
		{
			if (m_Next.get() != nullptr)
			{
				return m_Next->GetTableEnd();
			}
			else
			{
				return m_TableEnd;
			}
		}

		void Clear()
		{
			if (m_Next)
			{
				m_Next->Clear();
			}
			m_Next = nullptr;
		}
	};

	CpuDescriptorHandlesCache m_DescriptorHandlesCache[DX12MaxSlotsPerShader];
	uint64_t m_DynamicCbvSrvUavDescriptorsTableDirty;
};
