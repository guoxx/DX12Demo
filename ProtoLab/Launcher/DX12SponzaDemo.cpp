//
// DX12SponzaDemo.cpp
//

#include "pch.h"
#include "DX12SponzaDemo.h"

#include "3DEngine/Scene.h"
#include "3DEngine/Model.h"
#include "3DEngine/Camera.h"
#include "3DEngine/Lights/PointLight.h"
#include "3DEngine/Lights/DirectionalLight.h"
#include "3DEngine/Renderer.h"

#include "3DEngine/MaterialManager.h"
#include "3DEngine/RenderableSurfaceManager.h"
#include "3DEngine/GameInput.h"
#include "3DEngine/TextRenderer.h"
#include "3DEngine/EngineTuning.h"
#include "3DEngine/Spectrum.h"

#ifdef _XBOX_ONE
using namespace Windows::Xbox::Input;
using namespace Windows::Foundation::Collections;
#endif

DX12SponzaDemo::DX12SponzaDemo(uint32_t width, uint32_t height, std::wstring name)
	: DXSample(width, height, name)
{
}

// Initialize the Direct3D resources required to run.
void DX12SponzaDemo::OnInit(GFX_HWND hwnd)
{
	super::OnInit(hwnd);

    CreateDevice();
    CreateResources();
	LoadAssets();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

void DX12SponzaDemo::OnUpdate(DX::StepTimer const& timer)
{
    PIXBeginEvent(EVT_COLOR_UPDATE, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

	GameInput::Update(elapsedTime);
	EngineTuning::Update(elapsedTime);

	float speedScale = 2;
	float forward = (GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftStickY)) +
		(GameInput::IsPressed(GameInput::kKey_w) ? elapsedTime : 0.0f) +
		(GameInput::IsPressed(GameInput::kKey_s) ? -elapsedTime : 0.0f);
	float strafe = (GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftStickX)) +
		(GameInput::IsPressed(GameInput::kKey_d) ? elapsedTime : 0.0f) +
		(GameInput::IsPressed(GameInput::kKey_a) ? -elapsedTime : 0.0f);
	float ascent = (GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightTrigger)) -
		(GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogLeftTrigger)) +
		(GameInput::IsPressed(GameInput::kKey_e) ? elapsedTime : 0.0f) +
		(GameInput::IsPressed(GameInput::kKey_q) ? -elapsedTime : 0.0f);
	forward *= speedScale;
	strafe *= speedScale;
	ascent *= speedScale;
	m_Camera->Move(m_Camera->GetForward(), forward);
	m_Camera->Move(m_Camera->GetLeft(), strafe);
	m_Camera->Move(m_Camera->GetUp(), ascent);

	float panScale = 2 * DirectX::XM_1DIVPI * 180;
	float yaw = -(GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightStickX) + GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogMouseX) * 8);
	float pitch = GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogRightStickY) + GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogMouseY) * 8;
	float roll = 0.0f;
	yaw *= panScale;
	pitch *= panScale;
	m_Camera->RotatePitchYawRoll(pitch, yaw, roll);

    PIXEndEvent();
}

// Draws the scene.
void DX12SponzaDemo::OnRender()
{
    // Don't try to render anything before the first Update.
    if (m_Timer.GetFrameCount() == 0)
    {
        return;
    }

    PIXBeginEvent(EVT_COLOR_RENDER, L"Render");

	DrawScene();

    PIXEndEvent();
}

void DX12SponzaDemo::OnFlip()
{
    PIXBeginEvent(EVT_COLOR_PRESENT, L"Flip");

	m_Renderer->Flip();

	PIXEndEvent();
}

void DX12SponzaDemo::OnDestroy()
{
	DX12GraphicsManager::Finalize();
	MaterialManager::Finalize();
	RenderableSurfaceManager::Finalize();
	GameInput::Shutdown();
	TextRenderer::Shutdown();
	EngineTuning::Finalize();
}

void DX12SponzaDemo::DrawScene()
{
	m_Renderer->Render(m_Camera.get(), m_Scene.get());
}

#ifdef _XBOX_ONE
// Occurs when the game is being suspended.
void DX12SponzaDemo::OnSuspending()
{
	m_GraphicsManager->Suspend();
}

// Occurs when the game is resuming.
void DX12SponzaDemo::OnResuming()
{
	m_GraphicsManager->Resume();
    m_Timer.ResetElapsedTime();
}
#endif

// These are the resources that depend on the device.
void DX12SponzaDemo::CreateDevice()
{
	DX12GraphicsManager::Initialize({m_RenderDoc, m_DebugGfx});
	MaterialManager::Initialize();
	RenderableSurfaceManager::Initialize();
	GameInput::Initialize(m_Hwnd);
	TextRenderer::Initialize();
	EngineTuning::Initialize();
    Math::SampledSpectrum::Init();

	m_GraphicsManager = DX12GraphicsManager::GetInstance();
}

// Allocate all memory resources that change on a window SizeChanged event.
void DX12SponzaDemo::CreateResources()
{
	m_Renderer = std::make_shared<Renderer>(m_Hwnd, m_Width, m_Height);
}

void DX12SponzaDemo::LoadAssets()
{
	m_Camera = std::make_shared<Camera>();
	//m_Camera->LookAt(DirectX::XMVECTOR{0, 1000, 40}, DirectX::XMVECTOR{0, 0, 0}, DirectX::XMVECTOR{0, 1, 0});
	//m_Camera->LookAt(DirectX::XMVECTOR{100, 400, -40}, DirectX::XMVECTOR{0, 400, -40}, DirectX::XMVECTOR{0, 1, 0});
	m_Camera->LookAt(DirectX::XMVECTOR{1.0f, 4.0f, -0.4f}, DirectX::XMVECTOR{0.0f, 4.0f, -0.4f}, DirectX::XMVECTOR{0, 1, 0});
	m_Camera->SetViewParams(45, m_Width * 1.0f / m_Height, 0.1f, 50.0f);

	DX12ScopedGraphicsContext pGfxContext;

	m_Scene = std::make_shared<Scene>();
	std::vector<std::shared_ptr<Model>> models = Model::LoadFromFile(m_GraphicsManager->GetDevice(), pGfxContext.Get(), "Sponza/Sponza.fbx");
	//std::vector<std::shared_ptr<Model>> models = Model::LoadOBJ(m_GraphicsManager->GetDevice(), pGfxContext.Get(), "crytek-sponza/sponza.obj", "crytek-sponza/");
	//std::vector<std::shared_ptr<Model>> models = Model::LoadOBJ(m_GraphicsManager->GetDevice(), pGfxContext, "cornell-box/CornellBox-Glossy.obj", "cornell-box/");
	for (auto m : models)
	{
		m_Scene->AttachModel(m);
	}

	{
		std::shared_ptr<DirectionalLight> light = std::make_shared<DirectionalLight>();
		//light->SetDirection(0.58f, -0.75f, -0.31f);
		m_Scene->AttachDirectionalLight(light);
	}

	//{
	//	std::shared_ptr<PointLight> light = std::make_shared<PointLight>();
	//	light->SetRadius(3.2f);
 //       light->SetColor(1, 1, 1);
	//	light->SetRadiantPower(640000);
	//	light->SetTranslation(DirectX::XMVECTOR{ -4.0f, 1.8f, -1.0f, 0.0f });
	//	m_Scene->AttachPointLight(light);
	//}

	//{
	//	std::shared_ptr<PointLight> light = std::make_shared<PointLight>();
	//	light->SetRadius(3.2f);
 //       light->SetColor(1, 1, 1);
	//	light->SetRadiantPower(640000);
	//	light->SetTranslation(DirectX::XMVECTOR{ -12.0f, 1.8f, -2.5f, 0.0f });
	//	m_Scene->AttachPointLight(light);
	//}
}
