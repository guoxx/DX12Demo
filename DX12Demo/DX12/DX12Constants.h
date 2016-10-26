#pragma once

enum DX12ShaderType
{
	DX12ShaderTypeVertex = 0,
	DX12ShaderTypeGeometry,
	DX12ShaderTypePixel,
	DX12ShaderTypeCompute,
	DX12ShaderTypeMax,
};

enum DX12GpuResourceUsage
{
	DX12GpuResourceUsage_CpuReadable = 0x01,
	DX12GpuResourceUsage_CpuWritable = 0x02,
	DX12GpuResourceUsage_GpuReadable = 0x04,
	DX12GpuResourceUsage_GpuWritable = 0x08,

	DX12GpuResourceUsage_GpuReadOnly = DX12GpuResourceUsage_GpuReadable,
	DX12GpuResourceUsage_GpuReadWrite = DX12GpuResourceUsage_GpuReadable | DX12GpuResourceUsage_GpuWritable,
	DX12GpuResourceUsage_CpuWrite_GpuRead = DX12GpuResourceUsage_CpuWritable | DX12GpuResourceUsage_GpuReadable,
	DX12GpuResourceUsage_CpuRead_GpuWrite = DX12GpuResourceUsage_CpuReadable | DX12GpuResourceUsage_GpuWritable,
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

constexpr uint32_t DX12DirectionalLightShadowMapSize = 2048;
constexpr uint32_t DX12PointLightShadowMapSize = 1024;

constexpr D3D12_RESOURCE_STATES D3D12_RESOURCE_STATE_INVALID = (D3D12_RESOURCE_STATES)-1;