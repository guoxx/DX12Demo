//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "TextureTga.h"

using namespace DirectX;
using namespace Windows::Xbox::Input;
using namespace Windows::Foundation::Collections;

using Microsoft::WRL::ComPtr;

void CompileShaderFromFile(wchar_t* filepath, char* entry, char* profile, ID3DBlob** shaderBlob)
{
	ComPtr<ID3DBlob> errBlob;

	uint32_t compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	D3DCompileFromFile(filepath, nullptr, nullptr, entry, profile, compileFlags, 0, shaderBlob, &errBlob);
	if (errBlob != nullptr && errBlob->GetBufferPointer() > 0)
	{
		char* errMsg = static_cast<char*>(errBlob->GetBufferPointer());
		printf("%s\n", errMsg);
	}
}

Game::Game() :
    m_window(0),
    m_outputWidth(1920),
    m_outputHeight(1080),
    m_backBufferIndex(0),
    m_frame(0),
    m_fenceValues{}
{
	m_offsetX = 0;
}

Game::~Game()
{
	DX12GraphicManager::Finalize();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(IUnknown* window)
{
    m_window = window;

    CreateDevice();
    CreateResources();
	LoadAssets();
	LoadAssets1();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

// Executes the basic game loop.
void Game::Tick()
{
    PIXBeginEvent(EVT_COLOR_FRAME, L"Frame %I64u", m_frame);

    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();

    PIXEndEvent();
    m_frame++;
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(EVT_COLOR_UPDATE, L"Update");

    // Allow the game to exit by pressing the view button on a controller.
    // This is just a helper for development.
    IVectorView<IGamepad^>^ gamepads = Gamepad::Gamepads;

    for (unsigned i = 0; i < gamepads->Size; i++)
    {
        IGamepadReading^ reading = gamepads->GetAt(i)->GetCurrentReading();
        if (reading->IsViewPressed)
        {
            Windows::ApplicationModel::Core::CoreApplication::Exit();
        }
    }

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

	m_offsetX += 0.001;
	if (m_offsetX > 0.5)
	{
		m_offsetX = -0.5;
	}

    PIXEndEvent();
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    PIXBeginEvent(EVT_COLOR_RENDER, L"Render");

    // Prepare the command list to render a new frame.
    Clear();

	DrawScene();
	DrawScene1();

    Present();

    PIXEndEvent();
}

void Game::DrawScene()
{
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_commandList->SetGraphicsRoot32BitConstants(0, 1, &m_offsetX, 0);

	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->IASetIndexBuffer(&m_indexBufferView);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_PSO.Get());

	m_commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);
}

void Game::DrawScene1()
{
	m_commandList->SetGraphicsRootSignature(m_rootSignature1.Get());

	ID3D12DescriptorHeap* heaps[] = { m_srvHeap1.Get(), m_sampHeap1.Get() };
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap1->GetGPUDescriptorHandleForHeapStart());
	m_commandList->SetGraphicsRootDescriptorTable(1, m_sampHeap1->GetGPUDescriptorHandleForHeapStart());

	m_commandList->IASetVertexBuffers(0, 1, &m_vbView1);
	m_commandList->IASetIndexBuffer(&m_ibView1);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_PSO1.Get());

	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

// Helper method to prepare the command list for rendering and clear the back buffers.
void Game::Clear()
{
    // Reset command list and allocator.
    DX::ThrowIfFailed(m_commandAllocators[m_backBufferIndex]->Reset());
    DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].Get(), nullptr));

    // Transition the render target into the correct state to allow for drawing into it.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_commandList->ResourceBarrier(1, &barrier);

    // Clear the views.
    XMVECTORF32 clearColor;
    clearColor.v = XMColorSRGBToRGB(Colors::CornflowerBlue);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescriptor(m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
    m_commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    m_commandList->ClearRenderTargetView(rtvDescriptor, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight) };
    D3D12_RECT scissorRect = { 0, 0, m_outputWidth, m_outputHeight };
    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);
}

// Submits the command list to the GPU and presents the back buffer contents to the screen.
void Game::Present()
{
    // Transition the render target to the state that allows it to be presented to the display.
    D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    m_commandList->ResourceBarrier(1, &barrier);

    // Send the command list off to the GPU for processing.
    m_commandList->Close();
    m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));

    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    DX::ThrowIfFailed(m_swapChain->Present(1, 0));

    // Xbox One apps do not need to handle DXGI_ERROR_DEVICE_REMOVED or DXGI_ERROR_DEVICE_RESET.

    MoveToNextFrame();
}

// Occurs when the game is being suspended.
void Game::OnSuspending()
{
    m_commandQueue->SuspendX(0);

    // TODO: Save game progress using the ConnectedStorage API.
}

// Occurs when the game is resuming.
void Game::OnResuming()
{
    m_commandQueue->ResumeX();
    m_timer.ResetElapsedTime();

    // TODO: Handle changes in users and input devices.
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
	DX12GraphicManager::Initialize();

    // Create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    DX::ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_GRAPHICS_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));

    // Create descriptor heaps for render target views and depth stencil views.
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = c_swapBufferCount;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
    dsvDescriptorHeapDesc.NumDescriptors = 1;
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(m_rtvDescriptorHeap.ReleaseAndGetAddressOf())));
    DX::ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(m_dsvDescriptorHeap.ReleaseAndGetAddressOf())));

    m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // Create a command allocator for each back buffer that will be rendered to.
    for (UINT n = 0; n < c_swapBufferCount; n++)
    {
        DX::ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(m_commandAllocators[n].ReleaseAndGetAddressOf())));
    }

    // Create a command list for recording graphics commands.
    DX::ThrowIfFailed(m_d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_GRAPHICS_PPV_ARGS(m_commandList.ReleaseAndGetAddressOf())));
    m_commandList->Close();

    // Create a fence for tracking GPU execution progress.
    DX::ThrowIfFailed(m_d3dDevice->CreateFence(m_fenceValues[m_backBufferIndex], D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));
    m_fenceValues[m_backBufferIndex]++;

    m_fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE, nullptr));

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Wait until all previous GPU work is complete.
    WaitForGpu();

    // Release resources that are tied to the swap chain and update fence values.
    for (UINT n = 0; n < c_swapBufferCount; n++)
    {
        m_renderTargets[n].Reset();
        m_fenceValues[n] = m_fenceValues[m_backBufferIndex];
    }

    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D32_FLOAT;
    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        DX::ThrowIfFailed(m_swapChain->ResizeBuffers(c_swapBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0));

        // Xbox One apps do not need to handle DXGI_ERROR_DEVICE_REMOVED or DXGI_ERROR_DEVICE_RESET.
    }
    else
    {
        // First, retrieve the underlying DXGI device from the D3D device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory2> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_GRAPHICS_PPV_ARGS(dxgiFactory.GetAddressOf())));

        // Create a descriptor for the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = backBufferWidth;
        swapChainDesc.Height = backBufferHeight;
        swapChainDesc.Format = backBufferFormat;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = c_swapBufferCount;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.Flags = DXGIX_SWAP_CHAIN_MATCH_XBOX360_AND_PC;

        // Create a swap chain for the window.
        DX::ThrowIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
            m_d3dDevice.Get(),
            m_window,
            &swapChainDesc,
            nullptr,
            m_swapChain.ReleaseAndGetAddressOf()
            ));
    }

    // Obtain the back buffers for this window which will be the final render targets
    // and create render target views for each of them.
    for (UINT n = 0; n < c_swapBufferCount; n++)
    {
        DX::ThrowIfFailed(m_swapChain->GetBuffer(n, IID_GRAPHICS_PPV_ARGS(m_renderTargets[n].GetAddressOf())));

        WCHAR name[25];
        if (swprintf_s(name, L"Render target %u", n) > 0)
        {
            m_renderTargets[n]->SetName(name);
        }

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), n, m_rtvDescriptorSize);
        m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvDescriptor);
    }

    // Reset the index to the current back buffer.
    m_backBufferIndex = 0;

    // Allocate a 2-D surface as the depth/stencil buffer and create a depth/stencil view
    // on this surface.
    CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(depthBufferFormat, backBufferWidth, backBufferHeight);
    depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE depthOptimizedClearValue(depthBufferFormat, 1.0f, 0);

    DX::ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
        &depthHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &depthStencilDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_GRAPHICS_PPV_ARGS(m_depthStencil.ReleaseAndGetAddressOf())
        ));

    m_depthStencil->SetName(L"Depth stencil");

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthBufferFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // TODO: Initialize windows-size dependent objects here.
}

void Game::LoadAssets()
{
    DX::ThrowIfFailed(m_commandAllocators[m_backBufferIndex]->Reset());
    DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].Get(), nullptr));

	D3D12_ROOT_PARAMETER params[1];
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	params[0].Constants.ShaderRegister = 0;
	params[0].Constants.RegisterSpace = 0;
	params[0].Constants.Num32BitValues = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;	
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pParameters = params;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errBlob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errBlob);
	m_d3dDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf()));

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	CompileShaderFromFile(L"HelloTriangle.hlsl", "VSMain", "vs_5_0", &vertexShader);
	CompileShaderFromFile(L"HelloTriangle.hlsl", "PSMain", "ps_5_0", &pixelShader);

	D3D12_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { inputElemDesc, _countof(inputElemDesc) };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC  psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC( D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_PSO.ReleaseAndGetAddressOf()));

	struct Vertex
	{
		DirectX::XMFLOAT3 m_Position;
	};
	Vertex verts[] = {
		{ DirectX::XMFLOAT3(-0.5, -0.5, 0) },
		{ DirectX::XMFLOAT3( 0.5, -0.5, 0) },
		{ DirectX::XMFLOAT3( 0.0,  0.5, 0) },
	};
	uint32_t indices[] = { 0, 2, 1 };

	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(verts), 0 }),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> vertexBufferUploadHeap;
	void* vertexBufferUploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(verts), 0 }),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(vertexBufferUploadHeap.ReleaseAndGetAddressOf()));
	vertexBufferUploadHeap->Map(0, nullptr, &vertexBufferUploadPtr);
	std::memcpy(vertexBufferUploadPtr, verts, sizeof(verts));
	vertexBufferUploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_vertexBuffer.Get(), vertexBufferUploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vertexBufferView = D3D12_VERTEX_BUFFER_VIEW{ m_vertexBuffer->GetGPUVirtualAddress(), sizeof(verts), sizeof(verts[0]) };

	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(indices), 0 }),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_indexBuffer.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> indexBufferUploadHeap;
	void* indexBufferUploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(indices), 0 }),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(indexBufferUploadHeap.ReleaseAndGetAddressOf()));
	indexBufferUploadHeap->Map(0, nullptr, &indexBufferUploadPtr);
	std::memcpy(indexBufferUploadPtr, indices, sizeof(indices));
	indexBufferUploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_indexBuffer.Get(), indexBufferUploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_indexBufferView = D3D12_INDEX_BUFFER_VIEW{ m_indexBuffer->GetGPUVirtualAddress(), sizeof(indices), DXGI_FORMAT_R32_UINT };

	m_commandList->Close();
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	WaitForGpu();
}

void Game::LoadAssets1()
{
    DX::ThrowIfFailed(m_commandAllocators[m_backBufferIndex]->Reset());
    DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocators[m_backBufferIndex].Get(), nullptr));

	D3D12_DESCRIPTOR_RANGE descriptorRange0[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0, 0 },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRange1[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, 0 },
	};

	D3D12_ROOT_PARAMETER params[2];
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	params[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange0);
	params[0].DescriptorTable.pDescriptorRanges = descriptorRange0;
	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	params[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange1);
	params[1].DescriptorTable.pDescriptorRanges = descriptorRange1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;	
	rootSignatureDesc.NumParameters = _countof(params);
	rootSignatureDesc.pParameters = params;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rootSignatureBlob;
	ComPtr<ID3DBlob> errBlob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignatureBlob, &errBlob);
	m_d3dDevice->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(m_rootSignature1.ReleaseAndGetAddressOf()));

	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	CompileShaderFromFile(L"HelloTexture.hlsl", "VSMain", "vs_5_0", &vertexShader);
	CompileShaderFromFile(L"HelloTexture.hlsl", "PSMain", "ps_5_0", &pixelShader);

	D3D12_INPUT_ELEMENT_DESC inputElemDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXID", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { inputElemDesc, _countof(inputElemDesc) };

	D3D12_GRAPHICS_PIPELINE_STATE_DESC  psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature1.Get();
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
	m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_PSO1.ReleaseAndGetAddressOf()));

	struct Vertex
	{
		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT2 m_UV;
		uint32_t m_TexID;
	};
	Vertex verts[] = {
		{ DirectX::XMFLOAT3(-0.5, -0.5, 0), DirectX::XMFLOAT2(0, 1), 0 },
		{ DirectX::XMFLOAT3(-0.5,  0.5, 0), DirectX::XMFLOAT2(0, 0), 0 },
		{ DirectX::XMFLOAT3( 0.5, -0.5, 0), DirectX::XMFLOAT2(1, 1), 0 },

		{ DirectX::XMFLOAT3( 0.5,  0.5, 0), DirectX::XMFLOAT2(1, 0), 1 },
		{ DirectX::XMFLOAT3( 0.5, -0.5, 0), DirectX::XMFLOAT2(1, 1), 0 },
		{ DirectX::XMFLOAT3(-0.5,  0.5, 0), DirectX::XMFLOAT2(0, 0), 0 },
	};
	uint32_t indices[] = { 0, 1, 2, 3, 4, 5 };

	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(verts), 0 }),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_vertexBuffer1.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> vertexBufferUploadHeap;
	void* vertexBufferUploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(verts), 0 }),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(vertexBufferUploadHeap.ReleaseAndGetAddressOf()));
	vertexBufferUploadHeap->Map(0, nullptr, &vertexBufferUploadPtr);
	std::memcpy(vertexBufferUploadPtr, verts, sizeof(verts));
	vertexBufferUploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_vertexBuffer1.Get(), vertexBufferUploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer1.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	m_vbView1 = D3D12_VERTEX_BUFFER_VIEW{ m_vertexBuffer1->GetGPUVirtualAddress(), sizeof(verts), sizeof(verts[0]) };

	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(indices), 0 }),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_indexBuffer1.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> indexBufferUploadHeap;
	void* indexBufferUploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(D3D12_RESOURCE_ALLOCATION_INFO{ sizeof(indices), 0 }),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(indexBufferUploadHeap.ReleaseAndGetAddressOf()));
	indexBufferUploadHeap->Map(0, nullptr, &indexBufferUploadPtr);
	std::memcpy(indexBufferUploadPtr, indices, sizeof(indices));
	indexBufferUploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_indexBuffer1.Get(), indexBufferUploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer1.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	m_ibView1 = D3D12_INDEX_BUFFER_VIEW{ m_indexBuffer1->GetGPUVirtualAddress(), sizeof(indices), DXGI_FORMAT_R32_UINT };

	TextureReaderTga texReader0{ };
	texReader0.LoadTga("tex_0.tga");
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texReader0.GetWidth(), texReader0.GetHeight()),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_texture0.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> texture0UploadHeap;
	void* texture0UploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texReader0.GetWidth(), texReader0.GetHeight()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(texture0UploadHeap.ReleaseAndGetAddressOf()));
	texture0UploadHeap->Map(0, nullptr, &texture0UploadPtr);
	std::memcpy(texture0UploadPtr, texReader0.GetData(), texReader0.GetDataSize());
	texture0UploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_texture0.Get(), texture0UploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture0.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	TextureReaderTga texReader1{ };
	texReader1.LoadTga("tex_1.tga");
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texReader1.GetWidth(), texReader1.GetHeight()),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(m_texture1.ReleaseAndGetAddressOf()));

	ComPtr<ID3D12Resource> texture1UploadHeap;
	void* texture1UploadPtr = nullptr;
	m_d3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, texReader1.GetWidth(), texReader1.GetHeight()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_GRAPHICS_PPV_ARGS(texture1UploadHeap.ReleaseAndGetAddressOf()));
	texture1UploadHeap->Map(0, nullptr, &texture1UploadPtr);
	std::memcpy(texture1UploadPtr, texReader1.GetData(), texReader1.GetDataSize());
	texture1UploadHeap->Unmap(0, nullptr);
	m_commandList->CopyResource(m_texture1.Get(), texture1UploadHeap.Get());
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_texture1.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_GRAPHICS_PPV_ARGS(m_srvHeap1.ReleaseAndGetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC sampHeapDesc = {};
	sampHeapDesc.NumDescriptors = 1;
	sampHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	sampHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_d3dDevice->CreateDescriptorHeap(&sampHeapDesc, IID_GRAPHICS_PPV_ARGS(m_sampHeap1.ReleaseAndGetAddressOf()));

	m_cbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_samplerDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	D3D12_CPU_DESCRIPTOR_HANDLE texSrvHandle = m_srvHeap1->GetCPUDescriptorHandleForHeapStart();
	m_d3dDevice->CreateShaderResourceView(m_texture0.Get(), nullptr, texSrvHandle);
	texSrvHandle.ptr += m_cbvSrvUavDescriptorSize;
	m_d3dDevice->CreateShaderResourceView(m_texture1.Get(), nullptr, texSrvHandle);

	D3D12_CPU_DESCRIPTOR_HANDLE sampHandle = m_sampHeap1->GetCPUDescriptorHandleForHeapStart();
	D3D12_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D12_FILTER_ANISOTROPIC;
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.MipLODBias = 0;
    sampDesc.MaxAnisotropy = 16;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = 16;
	m_d3dDevice->CreateSampler(&sampDesc, sampHandle);

	m_commandList->Close();
	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	WaitForGpu();
}

void Game::WaitForGpu()
{
    // Schedule a Signal command in the GPU queue.
    DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_fenceValues[m_backBufferIndex]));

    // Wait until the Signal has been processed.
    DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
    WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);

    // Increment the fence value for the current frame.
    m_fenceValues[m_backBufferIndex]++;
}

void Game::MoveToNextFrame()
{
    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = m_fenceValues[m_backBufferIndex];
    DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), currentFenceValue));

    // Update the back buffer index.
    m_backBufferIndex = (m_backBufferIndex + 1) % c_swapBufferCount;

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (m_fence->GetCompletedValue() < m_fenceValues[m_backBufferIndex])
    {
        DX::ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_backBufferIndex], m_fenceEvent.Get()));
        WaitForSingleObjectEx(m_fenceEvent.Get(), INFINITE, FALSE);
    }

    // Set the fence value for the next frame.
    m_fenceValues[m_backBufferIndex] = currentFenceValue + 1;
}
