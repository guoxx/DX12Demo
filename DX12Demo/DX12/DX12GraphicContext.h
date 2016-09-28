#pragma once

#include "DX12CommandContext.h"


class DX12GpuResource;
class DX12RootSignature;
class DX12PipelineState;

class DX12GraphicContext : public DX12CommandContext
{
	using super = DX12CommandContext;
public:
	DX12GraphicContext(DX12Device* device);
	virtual ~DX12GraphicContext();

	void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW *pView);

	void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);

	void ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32 num32BitValuesToSet, const void *pSrcData, uint32 destOffsetIn32BitValues);

	void SetGraphicsRootSignature(DX12RootSignature* pRootSig);

	void SetPipelineState(DX12PipelineState* pPSO);

	void DrawIndexed(uint32 indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);
};
