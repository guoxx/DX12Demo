//
// DX12SponzaDemo.cpp
//

#include "pch.h"
#include "DX12SponzaDemo.h"

#include "3DEngine/Scene.h"
#include "3DEngine/Model.h"
#include "3DEngine/Camera.h"
#include "3DEngine/Renderer.h"

#include "3DEngine/MaterialManager.h"
#include "3DEngine/GameInput.h"

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

	GameInput::Update(elapsedTime);

	float speedScale = 50;
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
	DX12GraphicManager::Finalize();
	MaterialManager::Finalize();
	GameInput::Shutdown();
}

void DX12SponzaDemo::DrawScene()
{
	m_Renderer->Render(m_Camera.get(), m_Scene.get());
}

#ifdef _XBOX_ONE
// Occurs when the game is being suspended.
void DX12SponzaDemo::OnSuspending()
{
	m_GraphicManager->Suspend();
}

// Occurs when the game is resuming.
void DX12SponzaDemo::OnResuming()
{
	m_GraphicManager->Resume();
    m_Timer.ResetElapsedTime();
}
#endif

// These are the resources that depend on the device.
void DX12SponzaDemo::CreateDevice()
{
	DX12GraphicManager::Initialize();
	MaterialManager::Initialize();
	GameInput::Initialize(m_Hwnd);

	m_GraphicManager = DX12GraphicManager::GetInstance();

	m_GraphicManager->CreateGraphicCommandQueues();

}

// Allocate all memory resources that change on a window SizeChanged event.
void DX12SponzaDemo::CreateResources()
{
	m_Renderer = std::make_shared<Renderer>(m_Hwnd, m_Width, m_Height);
}

void DX12SponzaDemo::LoadAssets()
{
	m_Camera = std::make_shared<Camera>();
	m_Camera->LookAt(DirectX::XMVECTOR{0, 1, 2}, DirectX::XMVECTOR{0, 1, 0}, DirectX::XMVECTOR{0, 1, 0});
	m_Camera->SetViewParams(60, m_Width * 1.0f / m_Height, 0.1f, 5000.0f);

	DX12GraphicContextAutoExecutor executor;
	DX12GraphicContext* pGfxContext = executor.GetGraphicContext();

	m_Scene = std::make_shared<Scene>();
	std::vector<std::shared_ptr<Model>> models = Model::LoadOBJ(m_GraphicManager->GetDevice(), pGfxContext, "crytek-sponza/sponza.obj", "crytek-sponza/");
	//std::vector<std::shared_ptr<Model>> models = Model::LoadOBJ(m_GraphicManager->GetDevice(), pGfxContext, "cornell-box/CornellBox-Glossy.obj", "cornell-box/");
	for (auto m : models)
	{
		m_Scene->AttachModel(m);
	}
}
