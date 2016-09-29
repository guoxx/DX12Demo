#pragma once

class DX12GraphicContext;

class DX12GpuResource
{
	friend class DX12GraphicContext;

public:
	DX12GpuResource();
	DX12GpuResource(ComPtr<ID3D12Resource> resource);
	virtual ~DX12GpuResource();

	ID3D12Resource* GetGpuResource() const { return m_Resource.Get(); }

	void MapResource(uint32_t subresource, void** ppData);

	void UnmapResource(uint32_t subresource);

protected:
	ComPtr<ID3D12Resource> m_Resource;
};
