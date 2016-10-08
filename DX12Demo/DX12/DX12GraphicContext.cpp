#include "pch.h"
#include "DX12GraphicContext.h"

#include "DX12Device.h"
#include "DX12GpuResource.h"
#include "DX12Buffer.h"
#include "DX12ColorSurface.h"
#include "DX12DepthSurface.h"
#include "DX12RootSignature.h"
#include "DX12PipelineState.h"
#include "DX12DescriptorHandle.h"


DX12GraphicContext::DX12GraphicContext(DX12Device* device)
	: DX12CommandContext(device)
{
	m_CommandAllocator = device->CreateGraphicCommandAllocator();
	m_CommandList = device->CreateGraphicCommandList(m_CommandAllocator.Get());
}

DX12GraphicContext::~DX12GraphicContext()
{
}

void DX12GraphicContext::IASetIndexBuffer(const DX12IndexBuffer* pIndexBuffer)
{
	m_CommandList->IASetIndexBuffer(&pIndexBuffer->GetView());
}

void DX12GraphicContext::IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY primitiveTopology)
{
	m_CommandList->IASetPrimitiveTopology(primitiveTopology);
}

void DX12GraphicContext::ResourceTransitionBarrier(DX12GpuResource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, uint32_t subresource)
{
    // Transition the render target into the correct state to allow for drawing into it.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource->m_Resource.Get(), stateBefore, stateAfter, subresource);
    m_CommandList->ResourceBarrier(1, &barrier);
}

void DX12GraphicContext::SetGraphicsRoot32BitConstants(uint32_t rootParameterIndex, uint32_t num32BitValuesToSet, const void * pSrcData, uint32_t destOffsetIn32BitValues)
{
	m_CommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValuesToSet, pSrcData, destOffsetIn32BitValues);
}

void DX12GraphicContext::SetGraphicsRootStructuredBuffer(uint32_t rootParameterIndex, const DX12StructuredBuffer * pStructuredBuffer)
{
	m_CommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, pStructuredBuffer->GetGpuResource()->GetGPUVirtualAddress());
}

void DX12GraphicContext::SetGraphicsRootDescriptorTable(uint32_t rootParameterIndex, DX12DescriptorHandle baseDescriptorHandle)
{
	m_CommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptorHandle.GetGpuHandle());
}

void DX12GraphicContext::SetGraphicsRootSignature(DX12RootSignature * pRootSig)
{
	m_CommandList->SetGraphicsRootSignature(pRootSig->GetSignature());
}

void DX12GraphicContext::SetPipelineState(DX12PipelineState * pPSO)
{
	m_CommandList->SetPipelineState(pPSO->GetPSO());
}

void DX12GraphicContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	m_CommandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

void DX12GraphicContext::CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION * pDst, uint32_t dstX, uint32_t dstY, uint32_t dstZ, const D3D12_TEXTURE_COPY_LOCATION * pSrc, const D3D12_BOX * pSrcBox)
{
	m_CommandList->CopyTextureRegion(pDst, dstX, dstY, dstZ, pSrc, pSrcBox);
}

void DX12GraphicContext::CopyResource(DX12GpuResource* srcResource, DX12GpuResource* dstResource)
{
	m_CommandList->CopyResource(dstResource->GetGpuResource(), srcResource->GetGpuResource());
}

void DX12GraphicContext::ClearRenderTarget(DX12ColorSurface * pColorSurface, float r, float g, float b, float a)
{
    DirectX::XMVECTORF32 clearColor;
	clearColor.v = {r, g, b, a};

    m_CommandList->ClearRenderTargetView(pColorSurface->GetRTV().GetCpuHandle(), clearColor, 0, nullptr);

}

void DX12GraphicContext::ClearDepthTarget(DX12DepthSurface* pDepthSurface, float d)
{
    m_CommandList->ClearDepthStencilView(pDepthSurface->GetDSV().GetCpuHandle(), D3D12_CLEAR_FLAG_DEPTH, d, 0, 0, nullptr);
}

void DX12GraphicContext::SetRenderTargets(uint32_t numColorSurfaces, DX12ColorSurface * pColorSurface[], DX12DepthSurface * pDepthSurface)
{
	D3D12_CPU_DESCRIPTOR_HANDLE srvs[DX12MaxRenderTargetsCount];
	for (uint32_t i = 0; i < numColorSurfaces; ++i)
	{
		srvs[i] = pColorSurface[i]->GetRTV().GetCpuHandle();
	}
	m_CommandList->OMSetRenderTargets(numColorSurfaces, srvs, false, pDepthSurface != nullptr ? &pDepthSurface->GetDSV().GetCpuHandle() : nullptr);
}

void DX12GraphicContext::SetViewport(uint32_t topLeftX, uint32_t topLeftY, uint32_t width, uint32_t height)
{
    D3D12_VIEWPORT viewport = { (float)topLeftX, (float)topLeftY, (float)(width), (float)(height) };
    m_CommandList->RSSetViewports(1, &viewport);

	// TODO: remove this code
    D3D12_RECT scissorRect = { (float)topLeftX, (float)topLeftY, (float)(topLeftX + width), (float)(topLeftY + height) };
    m_CommandList->RSSetScissorRects(1, &scissorRect);
}
