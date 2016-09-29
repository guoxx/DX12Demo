#pragma once

#include "DXSample.h"
#include "DX12/DX12.h"

class DX12Demo : public DXSample
{
	using super = DXSample;

public:
	DX12Demo(uint32_t width, uint32_t height, std::wstring name);

	virtual void OnInit(GFX_WHND hwnd) override final;
	virtual void OnUpdate(DX::StepTimer const& timer) override final;
	virtual void OnRender() override final;
	virtual void OnFlip() override final;
	virtual void OnDestroy() override final;

#ifdef _XBOX_ONE
	virtual void OnSuspending() override final;
	virtual void OnResuming() override final;
#endif

	void DrawScene();
	void DrawScene1();

	// Rendering helpers
	void Clear();
	void Present();

private:
	void CreateDevice();
	void CreateResources();
	void LoadAssets();
	void LoadAssets1();

	DX12GraphicManager*									m_GraphicManager;
	std::shared_ptr<DX12SwapChain>						m_SwapChain;
	std::shared_ptr<DX12DepthSurface>					m_DepthSurface;
	std::shared_ptr<DX12RootSignature>					m_RootSig;
	std::shared_ptr<DX12PipelineState>					m_PSO;
	std::shared_ptr<DX12StructuredBuffer>				m_VertexBuffer;
	std::shared_ptr<DX12IndexBuffer>					m_IndexBuffer;

	float												m_offsetX;

	/*
	ComPtr<ID3D12RootSignature>							m_rootSignature1;
	ComPtr<ID3D12PipelineState>							m_PSO1;
	ComPtr<ID3D12Resource>								m_vertexBuffer1;
	ComPtr<ID3D12Resource>								m_indexBuffer1;
	D3D12_VERTEX_BUFFER_VIEW							m_vbView1;
	D3D12_INDEX_BUFFER_VIEW								m_ibView1;
	ComPtr<ID3D12Resource>								m_texture0;
	ComPtr<ID3D12Resource>								m_texture1;
	ComPtr<ID3D12DescriptorHeap>						m_srvHeap1;
	ComPtr<ID3D12DescriptorHeap>						m_sampHeap1;

	uint32_t                                            m_cbvSrvUavDescriptorSize;
	uint32_t                                            m_samplerDescriptorSize;
	*/
};
