#pragma once

enum DX12ShaderType
{
	DX12ShaderTypeVertex = 0,
	DX12ShaderTypePixel,
	DX12ShaderTypeCompute,
	DX12ShaderTypeMax,
};

constexpr uint32_t DX12MaxRenderTargetsCount = D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT;
constexpr uint32_t DX12NumGraphicContexts = 256;