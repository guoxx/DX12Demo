#include "pch.h"
#include "DX12Device.h"

DX12Device::DX12Device()
{
    DX::ThrowIfFailed(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_GRAPHICS_PPV_ARGS(m_d3dDevice.ReleaseAndGetAddressOf())));
}

DX12Device::~DX12Device()
{
}

ID3D12CommandQueue * DX12Device::CreateGraphicCommandQueue(int32_t priority, D3D12_COMMAND_QUEUE_FLAGS flags, uint32_t nodeMask)
{
	return nullptr;
}

ID3D12CommandAllocator * DX12Device::CreateGraphicCommandAllocator()
{
	return nullptr;
}

ID3D12GraphicsCommandList * DX12Device::CreateGraphicCommandList()
{
	return nullptr;
}

ID3D12Resource * DX12Device::CreateBuffer()
{
	return nullptr;
}

ID3D12Resource * DX12Device::CreateCommittedResourceInDefaultHeap(uint64_t sizeInBytes, uint64_t alignInBytes, D3D12_RESOURCE_STATES initialState)
{
	return nullptr;
}

ID3D12RootSignature * DX12Device::CreateRootSignature(const void * pBlobWithRootSignature, size_t blobLengthInBytes)
{
	return nullptr;
}

ID3D12PipelineState * DX12Device::CreateGraphicsPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC * desc)
{
	return nullptr;
}
