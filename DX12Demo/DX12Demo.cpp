//
// DX12Demo.cpp
//

#include "pch.h"
#include "DX12Demo.h"

#ifdef _XBOX_ONE
using namespace Windows::Xbox::Input;
using namespace Windows::Foundation::Collections;
#endif

DX12Demo::DX12Demo(uint32_t width, uint32_t height, std::wstring name)
	: DXSample(width, height, name)
{
	m_offsetX = 0;
}

// Initialize the Direct3D resources required to run.
void DX12Demo::OnInit(GFX_WHND hwnd)
{
	super::OnInit(hwnd);

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

void DX12Demo::OnUpdate(DX::StepTimer const& timer)
{
    PIXBeginEvent(EVT_COLOR_UPDATE, L"Update");

#ifdef _XBOX_ONE
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
#endif

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

	m_offsetX += 0.001f;
	if (m_offsetX > 0.5)
	{
		m_offsetX = -0.5;
	}

    PIXEndEvent();
}

// Draws the scene.
void DX12Demo::OnRender()
{
    // Don't try to render anything before the first Update.
    if (m_Timer.GetFrameCount() == 0)
    {
        return;
    }

    PIXBeginEvent(EVT_COLOR_RENDER, L"Render");

    // Prepare the command list to render a new frame.
    Clear();

	DrawScene();
	DrawScene1();


    PIXEndEvent();
}

void DX12Demo::OnFlip()
{
    Present();
}

void DX12Demo::OnDestroy()
{
	DX12GraphicManager::Finalize();
}

void DX12Demo::DrawScene()
{
	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->SetGraphicsRootSignature(m_RootSig.get());
	pGfxContext->SetGraphicsRoot32BitConstants(0, 1, &m_offsetX, 0);
	pGfxContext->SetGraphicsRootStructuredBuffer(1, m_VertexBuffer.get());

	pGfxContext->SetPipelineState(m_PSO.get());

	pGfxContext->IASetIndexBuffer(m_IndexBuffer.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	pGfxContext->DrawIndexed(3, 0, 0);
}

void DX12Demo::DrawScene1()
{
/*
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
*/
}

// Helper method to prepare the command list for rendering and clear the back buffers.
void DX12Demo::Clear()
{
	m_SwapChain->Begin();

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

    // Clear the views.
	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
    pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_DepthSurface.get());
    pGfxContext->ClearRenderTarget(pColorSurface, 0, 1, 1, 1);
	pGfxContext->ClearDepthTarget(m_DepthSurface.get(), 1.0f);
}

// Submits the command list to the GPU and presents the back buffer contents to the screen.
void DX12Demo::Present()
{
    PIXBeginEvent(EVT_COLOR_PRESENT, L"Present");

	m_SwapChain->Flip();

	PIXEndEvent();
}

#ifdef _XBOX_ONE
// Occurs when the game is being suspended.
void DX12Demo::OnSuspending()
{
	m_GraphicManager->Suspend();
}

// Occurs when the game is resuming.
void DX12Demo::OnResuming()
{
	m_GraphicManager->Resume();
    m_Timer.ResetElapsedTime();
}
#endif

// These are the resources that depend on the device.
void DX12Demo::CreateDevice()
{
	DX12GraphicManager::Initialize();
	m_GraphicManager = DX12GraphicManager::GetInstance();

	m_GraphicManager->CreateGraphicCommandQueues();

}

// Allocate all memory resources that change on a window SizeChanged event.
void DX12Demo::CreateResources()
{
	m_SwapChain = std::make_shared<DX12SwapChain>(m_GraphicManager->GetDevice(), m_Hwnd, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM);

	m_DepthSurface = std::make_shared<DX12DepthSurface>();
	m_DepthSurface->InitAs2dSurface(m_GraphicManager->GetDevice(), DXGI_FORMAT_D32_FLOAT, m_Width, m_Height);
}

void DX12Demo::LoadAssets()
{
	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2, 0);
	sigCompiler.End();
	sigCompiler[0].InitAsConstants(1, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	sigCompiler[1].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_RootSig = sigCompiler.Compile(m_GraphicManager->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromFile(DX12ShaderTypeVertex, L"HelloTriangle.hlsl", "VSMain");
	psoCompiler.SetShaderFromFile(DX12ShaderTypePixel, L"HelloTriangle.hlsl", "PSMain");
	psoCompiler.SetRoogSignature(m_RootSig.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
	m_PSO = psoCompiler.Compile(m_GraphicManager->GetDevice());

	struct Vertex
	{
		DirectX::XMFLOAT3 m_Position;
	};
	Vertex verts[] =
	{
		{ DirectX::XMFLOAT3(-0.5, -0.5, 0) },
		{ DirectX::XMFLOAT3( 0.5, -0.5, 0) },
		{ DirectX::XMFLOAT3( 0.0,  0.5, 0) },
	};
	uint32_t indices[] = { 0, 2, 1 };

	m_VertexBuffer = std::make_shared<DX12StructuredBuffer>(m_GraphicManager->GetDevice(), sizeof(verts), 0, sizeof(verts[0]));
	m_IndexBuffer = std::make_shared<DX12IndexBuffer>(m_GraphicManager->GetDevice(), sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_VertexBuffer.get(), verts, sizeof(verts));
	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void DX12Demo::LoadAssets1()
{
/*
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
#ifdef _XBOX_ONE
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
#endif
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
*/
}
