#pragma once

enum DX12ShaderType
{
	DX12ShaderTypeVertex = 0,
	DX12ShaderTypeGeometry,
	DX12ShaderTypePixel,
	DX12ShaderTypeCompute,
	DX12ShaderTypeMax,
};

constexpr uint32_t DX12NumSwapChainBuffers = 3;
constexpr uint32_t DX12MaxRenderTargetsCount = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
constexpr uint32_t DX12NumGraphicContexts = 256;
constexpr uint32_t DX12MaxFences = 2048;
constexpr uint64_t DX12UploadHeapSizeInBytes = 1024 * 1024 * 1024; // 1024MB
constexpr uint64_t DX12ConstantsBufferHeapSizeInBytes = 256* 1024 * 1024; // 256MB
constexpr uint64_t DX12MaxGraphicContextsInParallel = 8;
constexpr int32_t DX12ParallelIdInvalid = -1;

constexpr uint32_t DX12MaxSlotsPerShader = 32;
constexpr uint32_t DX12MaxElemsPerDescriptorTable = 64;

constexpr D3D12_RESOURCE_STATES D3D12_RESOURCE_STATE_INVALID = (D3D12_RESOURCE_STATES)-1;