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
void DX12Demo::OnInit(GFX_HWND hwnd)
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
	DX12SwapChainContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
	pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_DepthSurface.get());

	pGfxContext->SetGraphicsRootSignature(m_RootSig);
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
	DX12SwapChainContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	DX12ColorSurface* pColorSurface = m_SwapChain->GetBackBuffer();
	DX12ColorSurface* pColorSurfaces[] = { pColorSurface };
	pGfxContext->SetRenderTargets(_countof(pColorSurfaces), pColorSurfaces, m_DepthSurface.get());

	pGfxContext->SetGraphicsRootSignature(m_RootSig1);

	pGfxContext->SetGraphicsRootDescriptorTable(0, m_VertexBuffer1->GetDescriptorHandle());
	pGfxContext->SetGraphicsRootDescriptorTable(1, m_Tex1->GetSRV());

	pGfxContext->SetPipelineState(m_PSO1.get());

	pGfxContext->IASetIndexBuffer(m_IndexBuffer1.get());
	pGfxContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pGfxContext->SetViewport(0, 0, m_Width, m_Height);

	pGfxContext->DrawIndexed(6, 0, 0);
}

// Helper method to prepare the command list for rendering and clear the back buffers.
void DX12Demo::Clear()
{
	m_SwapChain->Begin();

	DX12SwapChainContextAutoExecutor executor;
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

	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_VertexBuffer.get(), verts, sizeof(verts));
	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_IndexBuffer.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer.get(), D3D12_RESOURCE_STATE_GENERIC_READ);
}

void DX12Demo::LoadAssets1()
{
	D3D12_DESCRIPTOR_RANGE descriptorRanges0[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0 },
	};
	D3D12_DESCRIPTOR_RANGE descriptorRanges1[] = {
		{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0 },
	};

	DX12RootSignatureCompiler sigCompiler;
	sigCompiler.Begin(2, 1);
	sigCompiler[0].InitAsDescriptorTable(_countof(descriptorRanges0), descriptorRanges0, D3D12_SHADER_VISIBILITY_VERTEX);
	sigCompiler[1].InitAsDescriptorTable(_countof(descriptorRanges1), descriptorRanges1, D3D12_SHADER_VISIBILITY_PIXEL);
	sigCompiler.InitStaticSampler(CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_ANISOTROPIC));
	sigCompiler.End();
	m_RootSig1 = sigCompiler.Compile(m_GraphicManager->GetDevice());

	DX12GraphicPsoCompiler psoCompiler;
	psoCompiler.SetShaderFromFile(DX12ShaderTypeVertex, L"HelloTexture.hlsl", "VSMain");
	psoCompiler.SetShaderFromFile(DX12ShaderTypePixel, L"HelloTexture.hlsl", "PSMain");
	psoCompiler.SetRoogSignature(m_RootSig1.get());
	psoCompiler.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	psoCompiler.SetDespthStencilFormat(DXGI_FORMAT_D32_FLOAT);
	m_PSO1 = psoCompiler.Compile(m_GraphicManager->GetDevice());

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

	m_VertexBuffer1 = std::make_shared<DX12StructuredBuffer>(m_GraphicManager->GetDevice(), sizeof(verts), 0, sizeof(verts[0]));
	m_IndexBuffer1 = std::make_shared<DX12IndexBuffer>(m_GraphicManager->GetDevice(), sizeof(indices), 0, DXGI_FORMAT_R32_UINT);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer1.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_VertexBuffer1.get(), verts, sizeof(verts));
	pGfxContext->ResourceTransitionBarrier(m_VertexBuffer1.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer1.get(), D3D12_RESOURCE_STATE_COPY_DEST);
	m_GraphicManager->UpdateBufer(pGfxContext, m_IndexBuffer1.get(), indices, sizeof(indices));
	pGfxContext->ResourceTransitionBarrier(m_IndexBuffer1.get(), D3D12_RESOURCE_STATE_GENERIC_READ);

	m_Tex1.reset(DX12Texture::LoadTGAFromFile(m_GraphicManager->GetDevice(), pGfxContext, "tex_0.tga"));
}
