#pragma once

#include "DXSample.h"

class DX12Demo : public DXSample
{
	using super = DXSample;

public:
	DX12Demo(uint32_t width, uint32_t height, std::wstring name);

	virtual void OnInit(GFX_WHND hwnd) override final;
	virtual void OnUpdate() override final;
	virtual void OnRender() override final;
	virtual void OnDestroy() override final;

#ifdef __XBOX_ONE__
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

	void WaitForGpu();
	void MoveToNextFrame();

	// Direct3D Objects
	D3D_FEATURE_LEVEL                                   m_featureLevel;
	static const uint32_t                                   c_swapBufferCount = 2;
	uint32_t                                                m_backBufferIndex;
	Microsoft::WRL::ComPtr<ID3D12Device>                m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>          m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
	uint32_t                                                m_rtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;
	uint32_t                                                m_dsvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      m_commandAllocators[c_swapBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Fence>                 m_fence;
	uint64_t                                              m_fenceValues[c_swapBufferCount];
	Microsoft::WRL::Wrappers::Event                     m_fenceEvent;

	// Rendering resources
	Microsoft::WRL::ComPtr<IDXGISwapChain1>             m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Resource>              m_renderTargets[c_swapBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource>              m_depthStencil;

	ComPtr<ID3D12RootSignature>							m_rootSignature;
	ComPtr<ID3D12PipelineState>							m_PSO;
	ComPtr<ID3D12Resource>								m_vertexBuffer;
	ComPtr<ID3D12Resource>								m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;

	float												m_offsetX;

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


	// Game state
	INT64                                               m_frame;
};

// PIX event colors
const DWORD EVT_COLOR_FRAME = PIX_COLOR_INDEX(1);
const DWORD EVT_COLOR_UPDATE = PIX_COLOR_INDEX(2);
const DWORD EVT_COLOR_RENDER = PIX_COLOR_INDEX(3);

