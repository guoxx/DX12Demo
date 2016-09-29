#pragma once

#include "DX12CommandContext.h"


class DX12GpuResource;
class DX12IndexBuffer;
class DX12ColorSurface;
class DX12DepthSurface;
class DX12RootSignature;
class DX12PipelineState;

class DX12GraphicContext : public DX12CommandContext
{
	using super = DX12CommandContext;
public:
	DX12GraphicContext(DX12Device* device);
	virtual ~DX12GraphicContext();

	void IASetIndexBuffer(const DX12IndexBuffer* pIndexBuffer);

	void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology);

	void ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

	void SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void *pSrcData, uint32_t destOffsetIn32BitValues);

	void SetGraphicsRootSignature(DX12RootSignature* pRootSig);

	void SetPipelineState(DX12PipelineState* pPSO);

	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);

	void CopyResource(DX12GpuResource* srcResource, DX12GpuResource* dstResource);

	void ClearRenderTarget(DX12ColorSurface* pColorSurface, float r, float g, float b, float a);

	void ClearDepthTarget(DX12DepthSurface* pDepthSurface, float d);

	void SetRenderTargets(uint32_t numColorSurfaces, DX12ColorSurface* pColorSurface[], DX12DepthSurface* pDepthSurface);

	void SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height);
};
