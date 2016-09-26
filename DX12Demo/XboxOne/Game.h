//
// Game.h
//

#pragma once

#include "../StepTimer.h"

// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game
{
public:

    Game();
	~Game();

    // Initialization and management
    void Initialize(IUnknown* window);

    // Basic game loop
    void Tick();
    void Render();
	void DrawScene();
	void DrawScene1();

    // Rendering helpers
    void Clear();
    void Present();

    // Messages
    void OnSuspending();
    void OnResuming();

private:

    void Update(DX::StepTimer const& timer);

    void CreateDevice();
    void CreateResources();
	void LoadAssets();
	void LoadAssets1();

    void WaitForGpu();
    void MoveToNextFrame();

    // Application state
    IUnknown*                                           m_window;
    int                                                 m_outputWidth;
    int                                                 m_outputHeight;

    // Direct3D Objects
    static const UINT                                   c_swapBufferCount = 2;
    UINT                                                m_backBufferIndex;
	Microsoft::WRL::ComPtr<ID3D12Device>                m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>          m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_rtvDescriptorHeap;
    UINT                                                m_rtvDescriptorSize;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>        m_dsvDescriptorHeap;
    UINT                                                m_dsvDescriptorSize;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>      m_commandAllocators[c_swapBufferCount];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>   m_commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence>                 m_fence;
    UINT64                                              m_fenceValues[c_swapBufferCount];
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
    DX::StepTimer                                       m_timer;
};

// PIX event colors
const DWORD EVT_COLOR_FRAME = PIX_COLOR_INDEX(1);
const DWORD EVT_COLOR_UPDATE = PIX_COLOR_INDEX(2);
const DWORD EVT_COLOR_RENDER = PIX_COLOR_INDEX(3);
